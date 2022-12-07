#include "socketserver.h"

#include <QTcpSocket>
#include <QDebug>
#include <QObject>


class Writer {
private:
    QByteArray buffer;

public:
    void pushAsk(const QString& name) {
        buffer.append("?");
        buffer.append(name.size());
        buffer.append(name.toUtf8());
    }
    void pushNameValue(const QString& name, const QString &value) {
        buffer.append("!");
        buffer.append(name.size());
        buffer.append(name.toUtf8());
        buffer.append(value.size());
        buffer.append(value.toUtf8());
    }
};

struct Field {
    QString name;
    QString value;
    bool isAsk;
    bool isValid;

    Field() : isValid(false) {}
    Field(const QString &name)
        : name(name)
        , isAsk(true)
        , isValid(true)
    {}
    Field(const QString &name, const QString &value)
        : name(name)
        , value(value)
        , isAsk(false)
        , isValid(true)
    {}
};

class Reader
{
private:
    QByteArray _buffer;
    int _index;

public:
    Reader(const QByteArray &buffer) : _buffer(buffer) {
        _index = 2;
    }

    void readName(Field &f)
    {
        int len = _buffer.at(_index);
        _index++;
        f.name = _buffer.mid(_index, len);
        _index += len;
    }

    void readValue(Field &f)
    {
        auto ch1 = _buffer.at(_index);
        auto ch2 = _buffer.at(_index + 1);
        auto ch3 = _buffer.at(_index + 2);
        auto ch4 = _buffer.at(_index + 3);
        auto len = (ch1 << 24) + (ch2 << 16) + (ch3 << 8) + ch4;

        _index += 4;
        f.value = _buffer.mid(_index, len);
        _index += len;
    }

    Field read() {
        if (_index >= _buffer.size())
            return {};

        auto type = _buffer.at(_index);
        _index++;

        Field f;
        if (type == '!') {
            f.isAsk = false;
            readName(f);
            readValue(f);
        } else if (type == '?'){
            f.isAsk = false;
            readName(f);
        } else {
            qDebug() << "Invalid index" << type;
            return {};
        }

        return f;
    }
};

bool operator==(const Field &f1, const Field &f2)
{
    return f1.isValid == f2.isValid && f1.name == f2.name;
}

bool operator!=(const Field &f1, const Field &f2)
{
    return !(f1== f2);
}

SocketServer::SocketServer() : QTcpServer()
{

}

void SocketServer::socket_readyRead()
{
    QStringList list{
        "\x00\x17",
        "?\x0ftarget_password",
        "?\x0btarget_host",
        "?\x0aproto_dest",
        "?\x08password",
        "?\x05login",
        "?\x0dtarget_device",
        "?\x0ctarget_login",
        "!\x03""bpp\x00\x00\x00\x02""24",
        "!\x05width\x00\x00\x00\x03""800",
        "!\x06height\x00\x00\x00\x03""600",
        "!\x15selector_current_page\x00\x00\x00\x01\x31",
        "!\x16selector_device_filter\x00\x00\x00\x00",
        "!\x15selector_group_filter\x00\x00\x00\x00",
        "!\x15selector_proto_filter\x00\x00\x00\x00",
        "!\x17selector_lines_per_page\x00\x00\x00\x01\x30",
        "!\x09reporting\x00\x00\x00\x00",
        "!\x13""auth_channel_target\x00\x00\x00\x00",
        "!\x0e""accept_message\x00\x00\x00\x05""False",
        "!\x0f""display_message\x00\x00\x00\x05""False",
        "!\x12real_target_device\x00\x00\x00\x00",
        "!\x09ip_client\x00\x00\x00\x00",
        "!\x09ip_target\x00\x00\x00\x00",
        "!\x0elogin_language\x00\x00\x00\x04""Auto"
    };
    auto socket = qobject_cast<QTcpSocket*>(sender());

    if (!socket)
        return;

    auto buffer = socket->readAll();
    Reader r(buffer);

    Field f;
    while ((f = r.read()) != Field()) {
        qDebug() << "Field" << f.name << f.value << f.isAsk;
    }

    qDebug() << buffer;//.split('\x0B');
}

void SocketServer::incomingConnection(qintptr handle)
{
    qDebug() << "New connection";
    auto socket = new QTcpSocket(this);
    socket->setSocketDescriptor(handle);


//    auto nextSocket = nextPendingConnection();

    connect(socket, &QTcpSocket::readyRead, this, &SocketServer::socket_readyRead);


}
