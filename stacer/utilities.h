#ifndef UTILITIES_H
#define UTILITIES_H

#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QWidget>

class Utilities
{
  public:
    static void
    addDropShadow(QWidget *widget, const int alpha, const int blur = 15)
    {
        addDropShadow(QList<QWidget *>() << widget, alpha, blur);
    }

    static void
    addDropShadow(QList<QWidget *> widgets, const int alpha, const int blur = 15)
    {
        for (QWidget *widget : widgets) {
            QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(widget);
            effect->setBlurRadius(blur);
            effect->setColor(QColor(0, 0, 0, alpha));
            effect->setOffset(0);
            widget->setGraphicsEffect(effect);
        }
    }

    static QString
    getDesktopValue(const QRegularExpression &val, const QStringList &lines)
    {
        QStringList filteredList = lines.filter(val);
        if (filteredList.count() > 0) {
            QString directive = filteredList.first().trimmed();
            QString command = directive.section('=', 1);
            return command.trimmed();
        }
        return QString("");
    }
};

#endif // UTILITIES_H
