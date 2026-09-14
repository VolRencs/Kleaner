// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QVariantList>

class QDBusPendingCallWatcher;

class ServiceBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)

  public:
    explicit ServiceBackend(QObject *parent = nullptr);

    bool available() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void setEnabled(const QString &unit, bool enabled);
    Q_INVOKABLE void start(const QString &unit);
    Q_INVOKABLE void stop(const QString &unit);
    Q_INVOKABLE void restart(const QString &unit);

  Q_SIGNALS:
    void availableChanged();
    void loaded(const QVariantList &services);
    void error(const QString &message);

  private:
    static bool detectSystemd();
    void refreshAvailability();
    void handleListUnitFiles(QDBusPendingCallWatcher *watcher, quint64 generation);
    void handleListUnits(QDBusPendingCallWatcher *watcher, quint64 generation);
    void finishReload(quint64 generation);
    void runUnitMethod(const QString &method, const QString &unit);

    bool m_available = false;

    quint64 m_generation = 0;
    bool m_unitFilesLoaded = false;
    bool m_unitsLoaded = false;
    QVariantList m_pendingServices;
};
