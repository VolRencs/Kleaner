// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "hosts.h"

#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QRegularExpression>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KJob>

namespace
{
const QRegularExpression &whitespacePattern()
{
    static const QRegularExpression pattern(QStringLiteral("\\s+"));
    return pattern;
}
}

Hosts::Hosts(QObject *parent) :
    QObject(parent)
{
    reload();
}

QVariantList Hosts::entriesProperty() const
{
    return m_entries;
}

void Hosts::reload()
{
    QFile file(QStringLiteral("/etc/hosts"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Keep the last known good content: clearing the entries here would let a
        // later save() overwrite /etc/hosts with a partial list.
        return;
    }

    const qint64 lastModified = QFileInfo(file).lastModified().toMSecsSinceEpoch();
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    if (file.error() != QFileDevice::NoError) {
        return;
    }

    m_lines = lines;
    m_hostLines.clear();

    for (int i = 0; i < m_lines.size(); ++i) {
        const QString trimmed = m_lines.at(i).trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
            continue;
        }
        const QStringList fields = trimmed.split(whitespacePattern(), Qt::SkipEmptyParts);
        if (fields.size() >= 2) {
            m_hostLines.insert(i);
        }
    }

    m_entries = parseEntries();
    m_loaded = true;
    m_lastModified = lastModified;
    Q_EMIT entriesChanged();
}

QVariantList Hosts::parseEntries() const
{
    QVariantList result;

    for (int i = 0; i < m_lines.size(); ++i) {
        const QString trimmed = m_lines.at(i).trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
            continue;
        }

        const QStringList fields = trimmed.split(whitespacePattern(), Qt::SkipEmptyParts);
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

void Hosts::setEntries(const QVariantList &entries)
{
    m_entries = entries;
    Q_EMIT entriesChanged();
}

void Hosts::save()
{
    if (!m_loaded) {
        Q_EMIT saved(false, QStringLiteral("/etc/hosts could not be read"));
        return;
    }

    const QFileInfo info(QStringLiteral("/etc/hosts"));
    if (info.exists() && m_lastModified > 0 && info.lastModified().toMSecsSinceEpoch() != m_lastModified) {
        Q_EMIT saved(false, QStringLiteral("/etc/hosts changed on disk, reload before saving"));
        return;
    }

    QMap<int, QString> replacements;
    QStringList appended;

    for (const QVariant &variant : std::as_const(m_entries)) {
        const QVariantMap entry = variant.toMap();
        const QString ip = entry.value(QStringLiteral("ip")).toString().trimmed();
        const QString names = entry.value(QStringLiteral("names")).toString().trimmed();

        // Incomplete rows keep their original line instead of being dropped.
        if (ip.isEmpty() || names.isEmpty() || ip.contains(QLatin1Char('\n')) || names.contains(QLatin1Char('\n'))) {
            continue;
        }

        const QString line = ip + QLatin1Char('\t') + names;

        bool lineOk = false;
        const int originalLine = entry.value(QStringLiteral("line")).toInt(&lineOk);
        if (lineOk && originalLine >= 0 && originalLine < m_lines.size()) {
            replacements.insert(originalLine, line);
        } else {
            appended.append(line);
        }
    }

    QStringList output;
    output.reserve(m_lines.size() + appended.size());
    for (int i = 0; i < m_lines.size(); ++i) {
        if (m_hostLines.contains(i)) {
            output.append(replacements.contains(i) ? replacements.value(i) : m_lines.at(i));
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
    job->start();
}
