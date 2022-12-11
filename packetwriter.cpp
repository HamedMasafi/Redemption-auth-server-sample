#include "packetwriter.h"

#include <QString>

PacketWriter::PacketWriter()
{

}

void PacketWriter::pushAsk(const QString &name) {
    _buffer.append("?");
    _buffer.append(name.size());
    _buffer.append(name.toUtf8());
    _count++;
}

void PacketWriter::pushNameValue(const QString &name, const QVariant &value) {
    _buffer.append("!");
    _buffer.append(name.size());
    _buffer.append(name.toUtf8());

    auto str = value.toString().toUtf8();
    auto n = str.size();
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
    _buffer.append(str);
    _count++;
}

QByteArray PacketWriter::createBuffer() const
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
