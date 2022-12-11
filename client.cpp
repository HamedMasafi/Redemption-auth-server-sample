#include "client.h"

Client::Client(QObject *parent)
    : QTcpSocket{parent}
{
    _status = Idle;
}

Client::Status Client::status() const
{
    return _status;
}

void Client::setStatus(Status newStatus)
{
    _status = newStatus;
}

QString Client::cert() const
{
    return _cert;
}

void Client::setCert(const QString &newCert)
{
    _cert = newCert;
}
