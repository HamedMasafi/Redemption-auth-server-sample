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
    r.print();
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
        w.pushAsk("target_device");
        w.pushAsk("proto_dest");
#endif
        w.pushAsk("login");
        w.pushAsk("ip_target");
        w.pushNameValue("module", "interactive_target");
        w.pushNameValue("selector", true);
//        w.pushNameValue("server_cert:server_cert_store", true);
//        w.pushNameValue("server_cert:server_access_allowed_message", 0x7);
//        w.pushNameValue("server_cert:server_cert_success_message", 0x7);
//        w.pushNameValue("server_cert:server_cert_create_message", 0x7);

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
//        w.pushNameValue("session_log_path", sessionId + ".log");
        w.pushNameValue("rt_display", false);
        w.pushNameValue("module", "RDP");
        w.pushNameValue("session_probe:enable_session_probe", false);
//        w.pushNameValue("enable_external_validation", true);

        w.pushNameValue("is_rec", true);
//        w.pushNameValue("rec_path", sessionId);

        w.pushNameValue("allow_channels", "*");
        w.pushNameValue("deny_channels", "");
        w.pushNameValue("pattern_kill", "$kbd:");
        w.pushNameValue("mod_rdp:mode_console", "allow");

//        w.pushNameValue("server_cert:server_cert_check", true);
//        w.pushNameValue("server_cert:server_cert_store", true);

        qDebug() << "[I] Client trying to connect; host=" << host
                 << "; username=" << r.fieldValue("target_login") << "; password=" << password;

        client->write(w.createBuffer());
        client->setStatus(Client::Connecting);
        break;
    }
    case Client::Connecting: {
        auto parts = reporting.split(":");

        if (r.fieldValue("module") == "close") {
            qDebug() << "[I] Session closed";
            return;
        }

        if (r.has("external_response")) {
            client->setCert(r.fieldValue("external_cert"));
            PacketWriter w;
//            w.pushNameValue("keepalive", true);
            w.pushNameValue("external_response", "Ok");
//            w.pushNameValue("external_cert", client->cert());
            client->write(w.createBuffer());
            qDebug() << "[I] Response to certificate";
            return;
        }
        PacketWriter w;
        w.pushNameValue("keepalive", true);
        client->write(w.createBuffer());

        client->setStatus(Client::Connected);
        qDebug() << "[I] Client connected to" << parts.at(1);
        break;
    }
    case Client::Connected: {
        auto parts = reporting.split(":");

        PacketWriter w;
        if (r.count() == 1 && r.at(0).name == "keepalive") {
            w.pushNameValue("keepalive", true);
            client->write(w.createBuffer());
            qDebug() << "[I] Send keepalive command";
            break;
        }

        if (parts.count() == 3) {
            if (parts.at(0) == "OPEN_SESSION_SUCCESSFUL") {
                qDebug() << "[I[ Session id is" << r.fieldValue("native_session_id");
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


