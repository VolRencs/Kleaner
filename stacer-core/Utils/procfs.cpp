#include "procfs.h"

#include <QFile>

namespace Procfs
{

QByteArray read(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

QList<QByteArray> lines(const QString &path)
{
    QList<QByteArray> result;
    const QByteArray content = read(path);
    if (content.isEmpty()) {
        return result;
    }
    const QList<QByteArray> rawLines = content.split('\n');
    result.reserve(rawLines.size());
    for (const QByteArray &line : rawLines) {
        if (!line.isEmpty()) {
            result.append(line);
        }
    }
    return result;
}

quint64 readUInt64(const QString &path, bool *ok)
{
    bool converted = false;
    const quint64 value = read(path).trimmed().toULongLong(&converted);
    if (ok) {
        *ok = converted;
    }
    return converted ? value : 0;
}

}
