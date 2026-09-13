#include "hosts.h"

#include <QFile>
#include <QMap>
#include <QRegularExpression>
#include <QSet>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KJob>

Hosts::Hosts(QObject *parent) :
    QObject(parent)
{
    reload();
}

void Hosts::reload()
{
    m_lines.clear();
    m_hostLines.clear();

    QFile file(QStringLiteral("/etc/hosts"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Q_EMIT loaded({});
        return;
    }

    m_lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));

    for (int i = 0; i < m_lines.size(); ++i) {
        const QString trimmed = m_lines.at(i).trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
            continue;
        }
        const QStringList fields = trimmed.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (fields.size() >= 2) {
            m_hostLines.insert(i);
        }
    }

    Q_EMIT loaded(entries());
}

QVariantList Hosts::entries() const
{
    QVariantList result;

    for (int i = 0; i < m_lines.size(); ++i) {
        const QString trimmed = m_lines.at(i).trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
            continue;
        }

        const QStringList fields = trimmed.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (fields.size() < 2) {
            continue;
        }

        result.append(QVariantMap {
            { QStringLiteral("line"), i },
            { QStringLiteral("ip"), fields.first() },
            { QStringLiteral("names"), QStringList(fields.mid(1)).join(QLatin1Char(' ')) },
        });
    }

    return result;
}

void Hosts::save(const QVariantList &entries)
{
    QMap<int, QString> replacements;
    QStringList appended;

    for (const QVariant &variant : entries) {
        const QVariantMap entry = variant.toMap();
        const QString ip = entry.value(QStringLiteral("ip")).toString().trimmed();
        const QString names = entry.value(QStringLiteral("names")).toString().trimmed();
        if (ip.isEmpty() || names.isEmpty()) {
            continue;
        }

        const QString line = ip + QLatin1Char('\t') + names;

        const int originalLine = entry.value(QStringLiteral("line")).toInt();
        if (originalLine >= 0 && originalLine < m_lines.size()) {
            replacements.insert(originalLine, line);
        } else {
            appended.append(line);
        }
    }

    QStringList output;
    output.reserve(m_lines.size() + appended.size());
    for (int i = 0; i < m_lines.size(); ++i) {
        if (m_hostLines.contains(i)) {
            if (replacements.contains(i)) {
                output.append(replacements.value(i));
            }
            continue;
        }
        output.append(m_lines.at(i));
    }

    output.append(appended);
    while (output.size() > 1 && output.last().isEmpty()) {
        output.removeLast();
    }

    const QString content = output.join(QLatin1Char('\n')) + QLatin1Char('\n');

    KAuth::Action action(QStringLiteral(KLEANER_APP_ID ".writehosts"));
    action.setHelperId(QStringLiteral(KLEANER_HELPER_ID));
    action.addArgument(QStringLiteral("content"), content.toUtf8());

    KAuth::ExecuteJob *job = action.execute();
    connect(job, &KJob::result, this, [this](KJob *kjob) {
        if (kjob->error() != KJob::NoError) {
            Q_EMIT saved(false, kjob->errorString());
        } else {
            reload();
            Q_EMIT saved(true, {});
        }
    });
}
