#include "socketserver.h"

#include <QTcpSocket>
#include <QDebug>
#include <QObject>
#include <QUuid>

#include "packetreader.h"
#include "packetwriter.h"
#include "field.h"
#include "client.h"

SocketServer::SocketServer() : QTcpServer()
{

}

void SocketServer::socket_readyRead()
{
    auto socket = qobject_cast<Client*>(sender());

    if (!socket)
        return;

    auto buffer = socket->readAll();
    Reader r(buffer);

    PacketWriter w;
    int index = 0;
    for (const auto &f: r) {
        qDebug() << "Field" << (++index) << f.name << f.value << f.isAsk;

        if (f.isAsk)
            w.pushAsk(f.name);
        else
            w.pushNameValue(f.name, f.value);
    }

    qDebug() << (w.createBuffer() == buffer);
    qDebug() << buffer; //.split('\x0B');
    qDebug() << w.createBuffer();

    switch (socket->status()) {
    case Client::Init: {
        PacketWriter w2;
        w2.pushAsk("target_password");
        w2.pushAsk("target_host");
        w2.pushAsk("login");
        w2.pushAsk("ip_target");
        w2.pushAsk("target_login");
        w2.pushNameValue("module", "interactive_target");
        w2.pushNameValue("mod_rdp:enable_session_probe", "False");
        socket->write(w2.createBuffer());
        socket->setStatus(Client::UsernamePassword);
        break;
    }
    case Client::UsernamePassword: {
        PacketWriter w2;
        auto host = r.field("target_host").value;
        auto password =  r.field("target_password").value;
        auto sessionId = QUuid::createUuid().toString(QUuid::Id128);
        w2.pushNameValue("target_password", password);
        w2.pushNameValue("target_host", host);
        w2.pushNameValue("password", password);
        w2.pushNameValue("ip_target", host);
        w2.pushNameValue("target_device", host);
        w2.pushNameValue("mod_rdp:enable_session_probe", "True");
        w2.pushNameValue("proto_dest", "RDP");
        w2.pushNameValue("module", "RDP");
        w2.pushNameValue("target_port", "3389");
        w2.pushNameValue("session_id", sessionId);
        w2.pushNameValue("rec_path", sessionId);
        w2.pushNameValue("session_log_path", sessionId + ".log");
        w2.pushNameValue("rt_display", "False");

        socket->write(w2.createBuffer());
        socket->setStatus(Client::UnKnown);
        break;
    }
    case Client::UnKnown:
        break;
    }
}

void SocketServer::incomingConnection(qintptr handle)
{
    qDebug() << "New connection";
    auto socket = new Client(this);
    socket->setSocketDescriptor(handle);

    connect(socket, &QTcpSocket::readyRead, this, &SocketServer::socket_readyRead);
}


