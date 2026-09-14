// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QString>

class SystemInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString hostname READ hostname CONSTANT)
    Q_PROPERTY(QString distribution READ distribution CONSTANT)
    Q_PROPERTY(QString kernel READ kernel CONSTANT)
    Q_PROPERTY(QString cpuModel READ cpuModel CONSTANT)
    Q_PROPERTY(QString username READ username CONSTANT)
    Q_PROPERTY(qulonglong uptimeSeconds READ uptimeSeconds NOTIFY changed)

  public:
    explicit SystemInfo(QObject *parent = nullptr);

    QString hostname() const;
    QString distribution() const;
    QString kernel() const;
    QString cpuModel() const;
    QString username() const;
    qulonglong uptimeSeconds() const;

    Q_INVOKABLE void update();

  Q_SIGNALS:
    void changed();

  private:
    void readOsRelease();
    void readCpuModel();

    QString m_hostname;
    QString m_distribution;
    QString m_kernel;
    QString m_cpuModel;
    QString m_username;
    qulonglong m_uptimeSeconds = 0;
};
