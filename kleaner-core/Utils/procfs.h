// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QByteArray>
#include <QList>
#include <QString>

namespace Procfs
{
[[nodiscard]] QByteArray read(const QString &path);
[[nodiscard]] QList<QByteArray> lines(const QString &path);
[[nodiscard]] quint64 readUInt64(const QString &path, bool *ok = nullptr);
}
