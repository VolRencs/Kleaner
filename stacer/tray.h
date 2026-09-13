#pragma once

#include <QObject>

class KStatusNotifierItem;

class Tray : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)

  public:
    explicit Tray(QObject *parent = nullptr);
    ~Tray() override;

    bool available() const;

  Q_SIGNALS:
    void showRequested();
    void quitRequested();

  private:
    KStatusNotifierItem *m_item = nullptr;
    bool m_available = false;
};
