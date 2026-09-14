// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

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
