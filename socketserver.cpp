#include "socketserver.h"

#include <QTcpSocket>
#include <QDebug>
#include <QObject>


class Writer {
private:
    QByteArray _buffer;
    int _count{0};

public:
    void pushAsk(const QString& name) {
        _buffer.append("?");
        _buffer.append(name.size());
        _buffer.append(name.toUtf8());
        _count++;
    }
    void pushNameValue(const QString& name, const QString &value) {
        _buffer.append("!");
        _buffer.append(name.size());
        _buffer.append(name.toUtf8());

        auto n = value.size();
        char ch1 = n & 0xFF;
        n >>= 8;
        char ch2 = n & 0xFF;
        n >>= 8;
        char ch3 = n & 0xFF;
        n >>= 8;
        char ch4 = n & 0xFF;
        _buffer.append(ch4);
        _buffer.append(ch3);
        _buffer.append(ch2);
        _buffer.append(ch1);
        _buffer.append(value.toUtf8());
        _count++;
    }
    QByteArray createBuffer() const;
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
        auto ch1 = buffer.at(0);
        auto ch2 = buffer.at(1);
        auto len = (ch2 << 8) + ch1;
        qDebug() << "header=" << len << QString::number(ch1) << QString::number(ch2) << "*";
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
            f.isAsk = true;
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
    auto socket = qobject_cast<QTcpSocket*>(sender());

    if (!socket)
        return;

    auto buffer = socket->readAll();
    Reader r(buffer);

    Field f;
    Writer w;
    int index = 0;
    while ((f = r.read()) != Field()) {
        qDebug() << "Field" << (++index) << f.name << f.value << f.isAsk;

        if (f.isAsk)
            w.pushAsk(f.name);
        else
            w.pushNameValue(f.name, f.value);
    }


    qDebug() << (w.createBuffer() == buffer) ;
    qDebug()<< buffer; //.split('\x0B');
    qDebug()<< w.createBuffer();

    Writer w2;
    w2.pushAsk("target_password");
    w2.pushAsk("target_host");
    w2.pushAsk("login");
    w2.pushAsk("ip_target");
    w2.pushAsk("target_login");
    w2.pushNameValue("module", "interactive_target");
    w2.pushNameValue("mod_rdp:enable_session_probe", "False");
    socket->write(w2.createBuffer());
}

void SocketServer::incomingConnection(qintptr handle)
{
    qDebug() << "New connection";
    auto socket = new QTcpSocket(this);
    socket->setSocketDescriptor(handle);


//    auto nextSocket = nextPendingConnection();

    connect(socket, &QTcpSocket::readyRead, this, &SocketServer::socket_readyRead);


}


QByteArray Writer::createBuffer() const
{
    auto b = _buffer;
    auto n = _count;
    char ch1 = n & 0xFF;
    n >>= 8;
    char ch2 = n & 0xFF;
    b.prepend(ch1);
    b.prepend(ch2);
    return b;
}
