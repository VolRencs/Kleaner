// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

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
    // Clamp to ~100 years so the millisecond conversion cannot overflow.
    constexpr qulonglong maximumSeconds = 100ULL * 365 * 24 * 60 * 60;
    return KFormat().formatDuration(static_cast<qint64>(qMin(seconds, maximumSeconds)) * 1000, KFormat::HideSeconds);
}
