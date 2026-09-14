// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QList>
#include <QObject>
#include <QVariantList>

class CpuInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int coreCount READ coreCount NOTIFY changed)
    Q_PROPERTY(double usage READ usage NOTIFY changed)
    Q_PROPERTY(QVariantList coreUsages READ coreUsages NOTIFY changed)
    Q_PROPERTY(double load1 READ load1 NOTIFY changed)
    Q_PROPERTY(double load5 READ load5 NOTIFY changed)
    Q_PROPERTY(double load15 READ load15 NOTIFY changed)
    Q_PROPERTY(double clock READ clock NOTIFY changed)

  public:
    explicit CpuInfo(QObject *parent = nullptr);

    int coreCount() const;
    double usage() const;
    QVariantList coreUsages() const;
    double load1() const;
    double load5() const;
    double load15() const;
    double clock() const;

    Q_INVOKABLE void update();

  Q_SIGNALS:
    void changed();

  private:
    void updateClocks();
    void updateLoads();

    int m_coreCount = 1;
    double m_usage = 0.0;
    QList<double> m_coreUsages;
    double m_load1 = 0.0;
    double m_load5 = 0.0;
    double m_load15 = 0.0;
    double m_clock = 0.0;

    bool m_hasBaseline = false;
    QList<quint64> m_previousTotals;
    QList<quint64> m_previousIdles;
};
