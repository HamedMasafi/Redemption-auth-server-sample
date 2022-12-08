#include "packetreader.h"

#include <QDebug>

Reader::Reader(const QByteArray &buffer) : _buffer(buffer) {
    auto ch1 = buffer.at(0);
    auto ch2 = buffer.at(1);
    _count = (ch2 << 8) + ch1;

    _index = 2;

    Field f;
    while ((f = read()) != Field())
        _fields << f;
}

void Reader::readName(Field &f)
{
    int len = _buffer.at(_index);
    _index++;
    f.name = _buffer.mid(_index, len);
    _index += len;
}

void Reader::readValue(Field &f)
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

Field Reader::read() {
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

Field Reader::field(const QString &name) const
{
    auto i = std::find_if(_fields.begin(), _fields.end(), [&name](const Field &f) { return f.name == name; });
    if (i == _fields.end())
        return {};
    return _fields[std::distance(_fields.begin(), i)];
}

QString Reader::fieldValue(const QString &name) const
{
    auto i = std::find_if(_fields.begin(), _fields.end(), [&name](const Field &f) { return f.name == name; });
    if (i == _fields.end())
        return {};
    return _fields[std::distance(_fields.begin(), i)].value;
}

QList<Field>::ConstIterator Reader::constBegin()
{
    return _fields.constBegin();
}

QList<Field>::ConstIterator Reader::constEnd()
{
    return _fields.constEnd();
}

QList<Field>::Iterator Reader::begin()
{
    return _fields.begin();
}

QList<Field>::Iterator Reader::end()
{
    return _fields.end();
}

QByteArray Reader::buffer() const
{
    return _buffer;
}

void Reader::print() const
{
    int index{0};

    for (const auto &f: std::as_const(_fields)) {
        index++;

        if (f.isAsk)
            qDebug() << "[I]  * Field (ask)" << index << f.name;
        else
            qDebug() << "[I]  * Field (val)" << index << f.name << f.value;
    }
}

int Reader::count() const
{
    return _fields.count();
}

const Field &Reader::at(int n) const
{
    return _fields.at(n);
}

bool Reader::has(const QString &name) const
{
    return std::any_of(_fields.begin(), _fields.end(), [&name](const Field &f) { return f.name == name; });
}
