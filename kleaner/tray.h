// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

class QMenu;
class KStatusNotifierItem;

class Tray : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

  public:
    explicit Tray(QObject *parent = nullptr);
    ~Tray() override;

    bool available() const;
    bool enabled() const;
    void setEnabled(bool enabled);

  Q_SIGNALS:
    void showRequested();
    void quitRequested();
    void enabledChanged();

  private:
    KStatusNotifierItem *m_item = nullptr;
    bool m_available = false;
    bool m_enabled = true;
};
