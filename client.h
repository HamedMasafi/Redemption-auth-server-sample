#pragma once

#include <QTcpSocket>

class Client : public QTcpSocket
{
    Q_OBJECT
public:
    enum Status { Idle, UsernamePassword, Connecting, Connected, UnKnown };
    Q_ENUM(Status);

    explicit Client(QObject *parent = nullptr);

    Status status() const;

    void setStatus(Status newStatus);

private:
    Status _status;
};
