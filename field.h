#pragma once

#include <QByteArray>
#include <QString>

struct Field {
    QString name;
    QByteArray value;
    bool isAsk;
    bool isValid;

    Field();
    Field(const QString &name);
    Field(const QString &name, const QByteArray &value);
};


bool operator==(const Field &f1, const Field &f2);
bool operator!=(const Field &f1, const Field &f2);
