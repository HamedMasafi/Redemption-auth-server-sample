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

    Field field(const QString& name) const;
    QString fieldValue(const QString& name) const;

    QList<Field>::ConstIterator constBegin();
    QList<Field>::ConstIterator constEnd();

    QList<Field>::Iterator begin();
    QList<Field>::Iterator end();

    QByteArray buffer() const;

    void print() const;
    int count() const;
    const Field &at(int n) const;
    bool has(const QString &name) const;

private:
    Field read();
    QList<Field> _fields;
    QByteArray _buffer;
    int _index;
    int _count;
};

