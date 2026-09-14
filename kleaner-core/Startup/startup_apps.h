// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QVariantList>

struct StartupApp {
    QString path;
    QString name;
    QString comment;
    QString exec;
    QString icon;
    bool enabled = false;
    bool system = false;
};

class StartupApps : public QObject
{
    Q_OBJECT

  public:
    explicit StartupApps(QObject *parent = nullptr);

    [[nodiscard]] Q_INVOKABLE QVariantList load() const;
    Q_INVOKABLE bool setEnabled(const QString &path, bool enabled);
    Q_INVOKABLE bool save(const QString &path, const QString &name, const QString &comment, const QString &exec, const QString &icon);
    [[nodiscard]] Q_INVOKABLE QString create(const QString &name, const QString &comment, const QString &exec, const QString &icon);
    Q_INVOKABLE bool remove(const QString &path);

  Q_SIGNALS:
    void changed();

  private:
    static QString userAutostartDir();
    static StartupApp readEntry(const QString &path, bool system);
};
