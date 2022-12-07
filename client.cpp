#include "client.h"

Client::Client(QObject *parent)
    : QTcpSocket{parent}
{
    _status = Init;
}

Client::Status Client::status() const
{
    return _status;
}

void Client::setStatus(Status newStatus)
{
    _status = newStatus;
}