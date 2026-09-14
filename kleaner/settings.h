// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class KConfigGroup;

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString startPage READ startPage WRITE setStartPage NOTIFY changed)
    Q_PROPERTY(QString closeBehavior READ closeBehavior WRITE setCloseBehavior NOTIFY changed)
    Q_PROPERTY(bool useTray READ useTray WRITE setUseTray NOTIFY changed)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

  public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings() override;

    QString startPage() const;
    void setStartPage(const QString &startPage);

    QString closeBehavior() const;
    void setCloseBehavior(const QString &closeBehavior);

    bool useTray() const;
    void setUseTray(bool useTray);

    QString language() const;
    void setLanguage(const QString &language);

    Q_INVOKABLE QVariantList availableLanguages() const;

    Q_INVOKABLE void sync();

    static QStringList translationDirectories();

  Q_SIGNALS:
    void changed();
    void languageChanged();

  private:
    KConfigGroup *m_group = nullptr;
};
