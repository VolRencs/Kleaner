#pragma once

#include <QByteArray>
#include <QList>
#include <QString>

namespace Procfs
{
QByteArray read(const QString &path);
QList<QByteArray> lines(const QString &path);
quint64 readUInt64(const QString &path, bool *ok = nullptr);
}
