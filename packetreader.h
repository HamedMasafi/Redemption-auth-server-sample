#pragma once

#include <QByteArray>
#include <QList>
#include "field.h"

class Reader
{

public:
    Reader(const QByteArray &buffer);

    void readName(Field &f);
    void readValue(Field &f);
    Field read();

    Field field(const QString& name) const;

    QList<Field>::ConstIterator constBegin();
    QList<Field>::ConstIterator constEnd();

    QList<Field>::Iterator begin();
    QList<Field>::Iterator end();

private:
    QList<Field> _fields;
    QByteArray _buffer;
    int _index;
};

