#include "socketserver.h"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SocketServer server;
    auto ok = server.listen(QHostAddress::Any, 8008);
    qDebug() << "Starting server" << ok;

    if (!ok) {
        qFatal("Unable to start server");
    }

    return a.exec();
}
