#pragma once

#include <QByteArray>
#include <QVariant>

class PacketWriter {
private:
    QByteArray _buffer;
    int _count{0};

public:
    PacketWriter();
    void pushAsk(const QString& name);
    void pushNameValue(const QString& name, const QVariant &value);
    QByteArray createBuffer() const;
};
