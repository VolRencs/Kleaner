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
    // sizes maps an entry path to the size measured during the last scan, so the
    // freed amount is reported without rescanning the file system here.
    Q_INVOKABLE void clean(const QStringList &paths, const QVariantMap &sizes = {}, const QStringList &orphanPackages = {}, bool vacuumJournal = false);

    static qulonglong directorySize(const QString &path);

  Q_SIGNALS:
    void scanningChanged();
    void scanned(const QVariantList &categories);
    void cleaned(int count, qulonglong freedBytes, const QString &error);

  private:
    static QVariantList buildCategories();
    static QString trashDirectory();

    bool m_scanning = false;
};
