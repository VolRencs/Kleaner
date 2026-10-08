// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

#include <QtQmlIntegration/qqmlintegration.h>

class QDBusServiceWatcher;
class KStatusNotifierItem;

class Tray : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Tray)
    QML_SINGLETON
    QML_UNCREATABLE("Provided by the application")
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool startHidden READ startHidden CONSTANT)

  public:
    explicit Tray(QObject *parent = nullptr);

    [[nodiscard]] bool available() const;
    [[nodiscard]] bool enabled() const;
    [[nodiscard]] bool startHidden() const;
    void setEnabled(bool enabled);

  Q_SIGNALS:
    void showRequested();
    void quitRequested();
    void availableChanged();
    void enabledChanged();

  private:
    void setupItem();
    void setAvailable(bool available);

    KStatusNotifierItem *m_item = nullptr;
    QDBusServiceWatcher *m_watcher = nullptr;
    bool m_available = false;
    bool m_enabled = true;
    bool m_startHidden = false;
};
