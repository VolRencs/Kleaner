// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "procfs.h"

#include <QFile>

#include <limits>

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
    const QByteArray token = read(path).trimmed().split(' ').value(0);
    quint64 value = token.toULongLong(&converted);

    if (!converted) {
        bool doubleOk = false;
        const double decimal = token.toDouble(&doubleOk);
        constexpr double maxUInt64 = 18446744073709551616.0;
        if (doubleOk && decimal >= 0.0 && decimal < maxUInt64) {
            value = static_cast<quint64>(decimal);
            converted = true;
        }
    }

    if (ok) {
        *ok = converted;
    }
    return converted ? value : 0;
}

}
