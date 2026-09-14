// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QString>

class NetworkInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString interface READ interface NOTIFY changed)
    Q_PROPERTY(qulonglong rxBytes READ rxBytes NOTIFY changed)
    Q_PROPERTY(qulonglong txBytes READ txBytes NOTIFY changed)
    Q_PROPERTY(double rxRate READ rxRate NOTIFY changed)
    Q_PROPERTY(double txRate READ txRate NOTIFY changed)
    Q_PROPERTY(bool connected READ connected NOTIFY changed)

  public:
    explicit NetworkInfo(QObject *parent = nullptr);

    QString interface() const;
    qulonglong rxBytes() const;
    qulonglong txBytes() const;
    double rxRate() const;
    double txRate() const;
    bool connected() const;

    Q_INVOKABLE void update();

  Q_SIGNALS:
    void changed();

  private:
    QString resolveDefaultInterface() const;
    QString resolveFallbackInterface() const;

    QString m_interface;
    qulonglong m_rxBytes = 0;
    qulonglong m_txBytes = 0;
    double m_rxRate = 0.0;
    double m_txRate = 0.0;
    bool m_connected = false;

    qulonglong m_previousRx = 0;
    qulonglong m_previousTx = 0;
    bool m_hasBaseline = false;
    QElapsedTimer m_elapsed;
};
