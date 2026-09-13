#include "apt_source_tool.h"
#include "Utils/command_util.h"
#include "Utils/file_util.h"

#include <QRegularExpression>

static QString binaryType()
{
    return CommandUtil::isAptRpm() ? "rpm" : "deb";
}

static QString sourceType()
{
    return CommandUtil::isAptRpm() ? "rpm-src" : "deb-src";
}

bool AptSourceTool::checkSourceRepository()
{
    QDir sourceList(APT_SOURCES_LIST_D_PATH);

    bool isExists = sourceList.exists();

    return isExists;
}

void AptSourceTool::removeAPTSource(const APTSourcePtr aptSource)
{
    if (CommandUtil::isAptRpm() && CommandUtil::isExecutable("apt-repo")) {
        // Use apt-repo rm for ALT Linux
        QStringList args = { "rm", aptSource->source };
        CommandUtil::sudoExec("apt-repo", args);
    } else {
        changeSource(aptSource, nullptr);
    }
}

void AptSourceTool::addRepository(const QString &repository, const bool isSource)
{
    if (!repository.isEmpty()) {
        if (CommandUtil::isAptRpm() && CommandUtil::isExecutable("apt-repo")) {
            // Use apt-repo add for ALT Linux
            // Format: apt-repo add "rpm <uri> <suite> <components>"
            QString source = repository;
            if (isSource && !source.startsWith("rpm-src")) {
                // Convert rpm to rpm-src if source is requested
                source.replace(QRegularExpression("^rpm\\s"), "rpm-src ");
            }
            QStringList args = { "add", source };
            CommandUtil::sudoExec("apt-repo", args);
        } else {
            QStringList args = { "-y", repository };
            if (isSource) {
                args << "-s";
            }
            CommandUtil::sudoExec("add-apt-repository", args);
        }
    }
}

void AptSourceTool::changeSource(const APTSourcePtr aptSource, const APTSourcePtr newSource)
{
    if (aptSource->filePath.endsWith(".sources")) {
        QStringList sourceFileContent = FileUtil::readListFromFile(aptSource->filePath);
        QStringList updatedContent;
        QString entry;
        auto processEntry = [&](const QString &entry) {
            QStringList entryLines = entry.split('\n');
            QMap<QString, QString> fields;
            QList<QPair<int, QString>> commentLines;
            for (int idx = 0; idx < entryLines.size(); ++idx) {
                const QString &entryLine = entryLines[idx];
                if (entryLine.trimmed().startsWith('#')) {
                    commentLines.append(qMakePair(idx, entryLine));
                    continue;
                }
                int sep = entryLine.indexOf(':');
                if (sep > 0) {
                    QString key = entryLine.left(sep).trimmed();
                    QString value = entryLine.mid(sep + 1).trimmed();
                    fields[key] = value;
                }
            }
            QString typeStr = aptSource->isSource ? sourceType() : binaryType();
            QString currentSource = QString("%1 %2 %3 %4")
                                        .arg(typeStr)
                                        .arg(fields.value("URIs"))
                                        .arg(fields.value("Suites"))
                                        .arg(fields.value("Components"));
            if (currentSource == aptSource->source) {
                if (!newSource) {
                    return; // skip this entry (remove)
                } else {
                    fields["Types"] = newSource->isSource ? sourceType() : binaryType();
                    fields["URIs"] = newSource->uri.trimmed();
                    fields["Suites"] = newSource->suites.trimmed();
                    fields["Components"] = newSource->components.trimmed();
                    if (!newSource->isActive) {
                        fields["Enabled"] = "no";
                    } else {
                        fields.remove("Enabled");
                    }
                }
            }
            // Reconstruct entry, preserving initial fields order, multi-line Signed-By, and comments
            QSet<QString> handledFields;
            QStringList newEntryLines(entryLines.size(), QString());
            // Place comments at their original positions
            for (const auto &pair : commentLines) {
                newEntryLines[pair.first] = pair.second;
            }
            int insertIdx = 0;
            for (int i = 0; i < entryLines.size(); ++i) {
                if (!newEntryLines[i].isEmpty())
                    continue; // already filled with comment
                QString line = entryLines[i];
                int sep = line.indexOf(':');
                if (sep > 0) {
                    QString key = line.left(sep).trimmed();
                    if (key == "Signed-By" && fields.contains("Signed-By")) {
                        bool found = false;
                        for (int j = i; j < entryLines.size(); ++j) {
                            QString l = entryLines[j];
                            if (l.startsWith("Signed-By:")) {
                                found = true;
                                newEntryLines[j] = l;
                                handledFields.insert("Signed-By");
                            } else if (found) {
                                if (l.trimmed().isEmpty() || l.contains(":"))
                                    break;
                                newEntryLines[j] = l;
                            }
                        }
                        while (i + 1 < entryLines.size() && !entryLines[i + 1].trimmed().isEmpty() && !entryLines[i + 1].contains(":")) {
                            ++i;
                        }
                    } else if (fields.contains(key)) {
                        newEntryLines[i] = QString("%1: %2").arg(key).arg(fields.value(key));
                        handledFields.insert(key);
                    }
                }
            }
            // Add new/updated fields not present in original order
            for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
                if (!handledFields.contains(it.key())) {
                    if (it.key() == "Signed-By") {
                        continue;
                    }
                    // Find first empty slot or append
                    bool placed = false;
                    for (int i = 0; i < newEntryLines.size(); ++i) {
                        if (newEntryLines[i].isEmpty()) {
                            newEntryLines[i] = QString("%1: %2").arg(it.key()).arg(it.value());
                            placed = true;
                            break;
                        }
                    }
                    if (!placed) {
                        newEntryLines << QString("%1: %2").arg(it.key()).arg(it.value());
                    }
                }
            }
            // Remove trailing empty lines
            while (!newEntryLines.isEmpty() && newEntryLines.last().isEmpty()) {
                newEntryLines.removeLast();
            }
            updatedContent.append(newEntryLines.join('\n'));
            updatedContent.append(""); // blank line between entries
        };
        for (const QString &line : sourceFileContent) {
            if (line.trimmed().isEmpty()) {
                if (!entry.isEmpty()) {
                    processEntry(entry);
                    entry.clear();
                }
            } else {
                entry += line + '\n';
            }
        }
        if (!entry.isEmpty()) {
            processEntry(entry);
        }
        QStringList args = { aptSource->filePath };
        QByteArray data = updatedContent.join('\n').append('\n').toUtf8();

        // content is empty, remove the file
        if (data.length() <= 1) {
            CommandUtil::sudoExec("rm", args);
            return;
        }

        CommandUtil::sudoExec("tee", args, data);
    } else if (aptSource->filePath.endsWith(".list")) {
        QStringList sourceFileContent = FileUtil::readListFromFile(aptSource->filePath);

        // find line index
        int pos = -1;
        for (int i = 0; i < sourceFileContent.count(); ++i) {
            int currentPos = sourceFileContent[i].indexOf(aptSource->source);
            if (currentPos != -1) {
                pos = i;
                break;
            }
        }

        if (pos != -1) {
            if (!newSource) {
                sourceFileContent.removeAt(pos);
            } else {
                // Reconstruct the line from newSource fields
                QString line = newSource->isSource ? sourceType() : binaryType();
                if (!newSource->options.isEmpty()) {
                    line += " " + newSource->options;
                }
                line += " " + newSource->uri + " " + newSource->suites;
                if (!newSource->components.isEmpty()) {
                    line += " " + newSource->components;
                }
                if (!newSource->isActive) {
                    line = "# " + line;
                }
                sourceFileContent.replace(pos, line);
            }

            QStringList args = { aptSource->filePath };

            QByteArray data = sourceFileContent.join('\n').append('\n').toUtf8();

            // content is empty, remove the file
            if (data.length() <= 1) {
                CommandUtil::sudoExec("rm", args);
                return;
            }

            CommandUtil::sudoExec("tee", args, data);
        }
    }
}

void AptSourceTool::changeStatus(const APTSourcePtr aptSource, const bool status)
{
    // Create a copy of the source to modify
    APTSourcePtr newSource(new APTSource(*aptSource));
    newSource->isActive = status;
    changeSource(aptSource, newSource);
}

QList<APTSourcePtr> AptSourceTool::getSourceList()
{
    QList<APTSourcePtr> aptSourceList;

    QDir sourcesDir(APT_SOURCES_LIST_D_PATH);
    QFileInfoList infoList = sourcesDir.entryInfoList({ "*.list" }, QDir::Files, QDir::Time);
    infoList.append(QFileInfo(APT_SOURCES_LIST_PATH)); // original sources.list of debian

    // Add deb822 sources files
    QFileInfoList deb822List = sourcesDir.entryInfoList({ "*.sources" }, QDir::Files, QDir::Time);
    infoList.append(deb822List);

    for (const QFileInfo &info : infoList) {
        if (info.fileName().endsWith(".sources")) {
            // Parse deb822 format
            QStringList fileContent = FileUtil::readListFromFile(info.absoluteFilePath());
            QString entry;
            auto processEntry = [&](const QString &entry) {
                QMap<QString, QString> fields;
                for (const QString &entryLine : entry.split('\n')) {
                    int sep = entryLine.indexOf(':');
                    if (sep > 0) {
                        QString key = entryLine.left(sep).trimmed();
                        QString value = entryLine.mid(sep + 1).trimmed();
                        fields[key] = value;
                    }
                }
                QString types = fields.value("Types");
                if (types.contains(binaryType())) {
                    APTSourcePtr aptSource(new APTSource);
                    aptSource->filePath = info.absoluteFilePath();
                    aptSource->isSource = types.contains(sourceType());
                    aptSource->uri = fields.value("URIs");
                    aptSource->suites = fields.value("Suites");
                    aptSource->components = fields.value("Components");
                    aptSource->options = "";
                    aptSource->isActive = fields.value("Enabled", "yes").toLower() == "yes";
                    QString typeStr = aptSource->isSource ? sourceType() : binaryType();
                    aptSource->source = QString("%1 %2 %3 %4")
                                            .arg(typeStr)
                                            .arg(aptSource->uri)
                                            .arg(aptSource->suites)
                                            .arg(aptSource->components);
                    aptSourceList.append(aptSource);
                }
            };
            for (const QString &line : fileContent) {
                if (line.trimmed().isEmpty()) {
                    if (!entry.isEmpty()) {
                        processEntry(entry);
                        entry.clear();
                    }
                } else {
                    entry += line + '\n';
                }
            }
            if (!entry.isEmpty()) {
                processEntry(entry);
            }
        } else if (info.fileName().endsWith(".list")) {
            // For APT: "deb [arch=amd64] https://packages.microsoft.com/repos/code stable main"
            // or for APT-RPM: "rpm [p10] http://mirror.yandex.ru/altlinux/ p10/branch/x86_64-i586 classic"
            QStringList fileContent = FileUtil::readListFromFile(info.absoluteFilePath()).filter(QRegularExpression("^\\s{0,}#{0,}\\s{0,}" + binaryType()));

            for (const QString &line : fileContent) {
                QString cleanLine = line.trimmed();

                APTSourcePtr aptSource(new APTSource);
                aptSource->filePath = info.absoluteFilePath();

                aptSource->isActive = !cleanLine.startsWith(QChar('#'));

                cleanLine.remove('#'); // remove comment

                // if has options
                QRegularExpression regexOption("(\\s[\\[]+.*[\\]]+)");
                QRegularExpressionMatch match;
                if (cleanLine.indexOf(regexOption, 0, &match) != -1) {
                    if (match.capturedLength() > 0) {
                        aptSource->options = match.captured().trimmed();
                    }
                }
                // remove options
                cleanLine.remove(regexOption);

                QStringList sourceColumns = cleanLine.trimmed().split(QRegularExpression("\\s+"));
                bool isBinary = sourceColumns.first() == binaryType();
                bool isSource = sourceColumns.first() == sourceType();

                if ((isBinary || isSource) && sourceColumns.count() > 2) {
                    aptSource->isSource = isSource;
                    aptSource->uri = sourceColumns.at(1);
                    aptSource->suites = sourceColumns.at(2);
                    aptSource->components = sourceColumns.mid(3).join(' ');

                    aptSource->source = line.trimmed().remove('#').trimmed();

                    aptSourceList.append(aptSource);
                }
            }
        }
    }

    return aptSourceList;
}
