#pragma once

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QObject>
#include <QVariantList>

class ServiceBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)

  public:
    explicit ServiceBackend(QObject *parent = nullptr);
    ~ServiceBackend() override;

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
    void handleListUnitFiles(QDBusPendingCallWatcher *watcher);
    void handleListUnits(QDBusPendingCallWatcher *watcher);
    void finishReload();
    void runUnitMethod(const QString &method, const QString &unit);

    QDBusInterface *m_interface = nullptr;
    bool m_available = false;

    bool m_unitFilesLoaded = false;
    bool m_unitsLoaded = false;
    QVariantList m_pendingServices;
};
