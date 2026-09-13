#include "format.h"

#include <KFormat>

Format::Format(QObject *parent) :
    QObject(parent)
{
}

QString Format::bytes(qulonglong value) const
{
    return KFormat().formatByteSize(static_cast<double>(value), 1, KFormat::BinaryUnitDialect::DefaultBinaryDialect);
}

QString Format::percent(double value, int decimals) const
{
    return QString::number(value, 'f', qBound(0, decimals, 4)) + QLatin1Char('%');
}

QString Format::duration(qulonglong seconds) const
{
    return KFormat().formatDuration(seconds * 1000ULL, KFormat::HideSeconds);
}
