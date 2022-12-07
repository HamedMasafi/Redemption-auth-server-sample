#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <QTcpServer>



class SocketServer : public QTcpServer
{
    Q_OBJECT

public:
    SocketServer();

private slots:
    void socket_readyRead();

protected:
    void incomingConnection(qintptr handle);
};

#endif // SOCKETSERVER_H
