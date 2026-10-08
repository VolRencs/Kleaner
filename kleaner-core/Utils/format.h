// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

#include <QtQmlIntegration/qqmlintegration.h>

class Format : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Format)
    QML_SINGLETON
    QML_UNCREATABLE("Provided by the application")

  public:
    explicit Format(QObject *parent = nullptr);

    [[nodiscard]] Q_INVOKABLE QString bytes(qulonglong value) const;
    [[nodiscard]] Q_INVOKABLE QString percent(double value, int decimals = 1) const;
    [[nodiscard]] Q_INVOKABLE QString number(double value, int decimals = 2) const;
    [[nodiscard]] Q_INVOKABLE QString duration(qulonglong seconds) const;
};
