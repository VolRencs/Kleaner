// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QVariantList>

class DiskInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList disks READ disks NOTIFY changed)
    Q_PROPERTY(qulonglong readBytes READ readBytes NOTIFY changed)
    Q_PROPERTY(qulonglong writeBytes READ writeBytes NOTIFY changed)
    Q_PROPERTY(double readRate READ readRate NOTIFY changed)
    Q_PROPERTY(double writeRate READ writeRate NOTIFY changed)

  public:
    explicit DiskInfo(QObject *parent = nullptr);

    QVariantList disks() const;
    qulonglong readBytes() const;
    qulonglong writeBytes() const;
    double readRate() const;
    double writeRate() const;

    Q_INVOKABLE void update();

  Q_SIGNALS:
    void changed();

  private:
    QVariantList m_disks;

    qulonglong m_readBytes = 0;
    qulonglong m_writeBytes = 0;
    double m_readRate = 0.0;
    double m_writeRate = 0.0;

    qulonglong m_previousRead = 0;
    qulonglong m_previousWrite = 0;
    bool m_hasBaseline = false;
    QElapsedTimer m_elapsed;
};
