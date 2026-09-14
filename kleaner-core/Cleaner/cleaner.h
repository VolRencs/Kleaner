// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QVariantList>

class Cleaner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)

  public:
    explicit Cleaner(QObject *parent = nullptr);

    bool scanning() const;

    Q_INVOKABLE void scan();
    Q_INVOKABLE void clean(const QStringList &paths, const QStringList &orphanPackages = {}, bool vacuumJournal = false);

    static qulonglong directorySize(const QString &path);

  Q_SIGNALS:
    void scanningChanged();
    void scanned(const QVariantList &categories);
    void cleaned(int count, const QString &error);

  private:
    static QVariantList buildCategories();
    static QString trashDirectory();

    bool m_scanning = false;
};
