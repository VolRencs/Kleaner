// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include <KConfigGroup>

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString startPage READ startPage WRITE setStartPage NOTIFY changed)
    Q_PROPERTY(QString closeBehavior READ closeBehavior WRITE setCloseBehavior NOTIFY changed)
    Q_PROPERTY(bool useTray READ useTray WRITE setUseTray NOTIFY changed)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(int windowWidth READ windowWidth WRITE setWindowWidth NOTIFY changed)
    Q_PROPERTY(int windowHeight READ windowHeight WRITE setWindowHeight NOTIFY changed)

  public:
    explicit Settings(QObject *parent = nullptr);

    [[nodiscard]] QString startPage() const;
    void setStartPage(const QString &startPage);

    [[nodiscard]] QString closeBehavior() const;
    void setCloseBehavior(const QString &closeBehavior);

    [[nodiscard]] bool useTray() const;
    void setUseTray(bool useTray);

    [[nodiscard]] QString language() const;
    void setLanguage(const QString &language);

    [[nodiscard]] int windowWidth() const;
    void setWindowWidth(int width);

    [[nodiscard]] int windowHeight() const;
    void setWindowHeight(int height);

    [[nodiscard]] Q_INVOKABLE QVariantList availableLanguages() const;

    Q_INVOKABLE void sync();

    [[nodiscard]] static QStringList translationDirectories();

  Q_SIGNALS:
    void changed();
    void languageChanged();

  private:
    KConfigGroup m_group;
};
