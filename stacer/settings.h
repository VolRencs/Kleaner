#pragma once

#include <QObject>
#include <QString>

class KConfigGroup;

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString startPage READ startPage WRITE setStartPage NOTIFY changed)
    Q_PROPERTY(QString closeBehavior READ closeBehavior WRITE setCloseBehavior NOTIFY changed)
    Q_PROPERTY(bool useTray READ useTray WRITE setUseTray NOTIFY changed)

  public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings() override;

    QString startPage() const;
    void setStartPage(const QString &startPage);

    QString closeBehavior() const;
    void setCloseBehavior(const QString &closeBehavior);

    bool useTray() const;
    void setUseTray(bool useTray);

    Q_INVOKABLE void sync();

  Q_SIGNALS:
    void changed();

  private:
    KConfigGroup *m_group = nullptr;
};
