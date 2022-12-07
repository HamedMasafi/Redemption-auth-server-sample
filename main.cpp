#include "socketserver.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SocketServer server;
    qDebug() << "Starting server" << server.listen(QHostAddress::Any, 8008);

    return a.exec();
}
