#include "socketserver.h"

#include <QTcpSocket>
#include <QDebug>
#include <QObject>
#include <QUuid>

#include "packetreader.h"
#include "packetwriter.h"
#include "field.h"
#include "client.h"

#define SAMPLE_AUTH

SocketServer::SocketServer() : QTcpServer()
{

}

void testBuffer(Reader &r)
{
    PacketWriter w;
    int index = 0;
    for (const auto &f: r) {
        if (f.isAsk)
            w.pushAsk(f.name);
        else
            w.pushNameValue(f.name, f.value);
    }

    Q_ASSERT(w.createBuffer() == r.buffer());
}

void SocketServer::socket_readyRead()
{
    auto client = qobject_cast<Client *>(sender());

    if (!client)
        return;

    auto buffer = client->readAll();
    Reader r(buffer);

    testBuffer(r);

    auto reporting = r.fieldValue("reporting");

    if (reporting != QString{}) {
        auto parts = reporting.split(":");
        if (parts.at(0) == "CONNECTION_FAILED") {
            client->setStatus(Client::Idle);
            qDebug() << "[E] connection closed; reason=" << parts.at(2);
            return;
        }
    }

    switch (client->status()) {
    case Client::Idle: {
        PacketWriter w;

#ifdef SAMPLE_AUTH2
        w.pushNameValue("target_host", "213.32.14.68");
        w.pushNameValue("target_login", "administrator");
        w.pushNameValue("target_password", "evb3PrHwWhqn6rH9");
#else
        w.pushAsk("target_password");
        w.pushAsk("target_host");
        w.pushAsk("target_login");
#endif
        w.pushAsk("login");
        w.pushAsk("ip_target");
        w.pushNameValue("module", "interactive_target");
        w.pushNameValueBool("mod_rdp:enable_session_probe", false);

        client->write(w.createBuffer());

        qDebug() << "[I] Client initalization";

        client->setStatus(Client::UsernamePassword);
        break;
    }

    case Client::UsernamePassword: {
        PacketWriter w;
        auto host = r.fieldValue("target_host");
        auto password = r.fieldValue("target_password");
        auto sessionId = QUuid::createUuid().toString(QUuid::Id128);

        w.pushNameValue("target_host", host);
        w.pushNameValue("target_password", password);
        w.pushNameValue("target_device", host);
        w.pushNameValue("target_port", "3389");
        w.pushNameValue("ip_target", host);

        w.pushNameValue("password", password);
        w.pushNameValue("proto_dest", "RDP");
        w.pushNameValue("session_id", sessionId);
        w.pushNameValue("session_log_path", sessionId + ".log");
        w.pushNameValue("rt_display", "False");
        w.pushNameValue("module", "RDP");
        w.pushNameValue("mod_rdp:enable_session_probe", "True");

        w.pushNameValueBool("is_rec", true);
        w.pushNameValue("rec_path", sessionId);

        w.pushNameValue("allow_channels", "*");
        w.pushNameValue("deny_channels", "");
        w.pushNameValue("pattern_kill", "$kbd:");
        w.pushNameValue("", "");
        w.pushNameValue("", "");
        w.pushNameValue("", "");

        qDebug() << "[I] Client trying to connect; host=" << host
                 << "; username=" << r.fieldValue("target_login") << "; password=" << password;

        client->write(w.createBuffer());
        client->setStatus(Client::Connecting);
        break;
    }
    case Client::Connecting: {
        auto parts = reporting.split(":");

        PacketWriter w;
        w.pushNameValue("keepalive", "True");
        client->write(w.createBuffer());

        client->setStatus(Client::Connected);
        qDebug() << "[I] Client connected to" << r.fieldValue("target_host");
        break;
    }
    case Client::Connected: {
        auto parts = reporting.split(":");

        PacketWriter w;
        if (r.count() == 1 && r.at(0).name == "keepalive") {
            w.pushNameValue("keepalive", "True");
            client->write(w.createBuffer());
            qDebug() << "[I] Send keepalive command";
            break;
        }

        if (parts.count() == 3) {
            if (parts.at(0) == "OPEN_SESSION_SUCCESSFUL") {
                qDebug() << "Session id: " << r.fieldValue("native_session_id");
                break;
            }
        }
        qDebug() << "Unhandled conected";
        r.print();
        break;
    }

    default:
        qDebug() << "Unhandled status" << client->status();
        r.print();
        break;
    }
}

void SocketServer::socket_aboutToClose()
{
    qDebug() << "[W] Socket closed";
}

void SocketServer::incomingConnection(qintptr handle)
{
    qDebug() << "[I] New connection";
    auto socket = new Client(this);
    socket->setSocketDescriptor(handle);

    connect(socket, &QTcpSocket::readyRead, this, &SocketServer::socket_readyRead);
    connect(socket, &QTcpSocket::aboutToClose, this, &SocketServer::socket_aboutToClose);
}


