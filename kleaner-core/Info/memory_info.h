// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

class MemoryInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qulonglong total READ total NOTIFY changed)
    Q_PROPERTY(qulonglong used READ used NOTIFY changed)
    Q_PROPERTY(qulonglong available READ available NOTIFY changed)
    Q_PROPERTY(qulonglong swapTotal READ swapTotal NOTIFY changed)
    Q_PROPERTY(qulonglong swapUsed READ swapUsed NOTIFY changed)
    Q_PROPERTY(double usagePercent READ usagePercent NOTIFY changed)
    Q_PROPERTY(double swapPercent READ swapPercent NOTIFY changed)

  public:
    explicit MemoryInfo(QObject *parent = nullptr);

    qulonglong total() const;
    qulonglong used() const;
    qulonglong available() const;
    qulonglong swapTotal() const;
    qulonglong swapUsed() const;
    double usagePercent() const;
    double swapPercent() const;

    Q_INVOKABLE void update();

  Q_SIGNALS:
    void changed();

  private:
    qulonglong m_total = 0;
    qulonglong m_used = 0;
    qulonglong m_available = 0;
    qulonglong m_swapTotal = 0;
    qulonglong m_swapUsed = 0;
};
