#include "field.h"


Field::Field() : isValid(false) {}

Field::Field(const QString &name)
    : name(name)
    , isAsk(true)
    , isValid(true)
{}

Field::Field(const QString &name, const QString &value)
    : name(name)
    , value(value)
    , isAsk(false)
    , isValid(true)
{}

bool operator!=(const Field &f1, const Field &f2)
{
    return !(f1== f2);
}

bool operator==(const Field &f1, const Field &f2)
{
    return f1.isValid == f2.isValid && f1.name == f2.name;
}
