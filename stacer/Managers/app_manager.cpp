#include "app_manager.h"
#include <QLibraryInfo>
#include <QRegularExpression>

AppManager *AppManager::instance = nullptr;

AppManager *AppManager::ins()
{
    if (!instance) {
        instance = new AppManager;
    }

    return instance;
}

AppManager::AppManager()
{
    mSettingManager = SettingManager::ins();

    mTrayIcon = new QSystemTrayIcon(QIcon(":/static/logo.png"));

    loadLanguageList();

    loadThemeList();

    // Load from global qt translations path
    QString translationPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);

    // Prefer application installation path if available
    QString applicationTranslationPath = "/usr/share/stacer/translations";
    if (QDir(applicationTranslationPath).exists()) {
        translationPath = applicationTranslationPath;
    }

    // Flatpak translation path (highest priority for sandboxed build)
    QString flatpakTranslationPath = "/app/share/stacer/translations";
    if (QDir(flatpakTranslationPath).exists()) {
        translationPath = flatpakTranslationPath;
    }

    // Fallback to local translations path if global/application path does not exist
    QString folderTranslationPath = qApp->applicationDirPath() + "/translations";
    if (QDir(folderTranslationPath).exists()) {
        translationPath = folderTranslationPath;
    }

    if (mTranslator.load(QString("stacer_%1").arg(mSettingManager->getLanguage()), translationPath)) {
        qApp->installTranslator(&mTranslator);
        (mSettingManager->getLanguage() == "ar") ? qApp->setLayoutDirection(Qt::RightToLeft) : qApp->setLayoutDirection(Qt::LeftToRight);
    }
}

QSystemTrayIcon *AppManager::getTrayIcon()
{
    return mTrayIcon;
}

QSettings *AppManager::getStyleValues() const
{
    return mStyleValues;
}

void AppManager::loadLanguageList()
{
    QByteArray languagesJson = FileUtil::readStringFromFile(":/static/languages.json").toUtf8();
    QJsonArray languages = QJsonDocument::fromJson(languagesJson).array();

    for (int i = 0; i < languages.count(); ++i) {
        QJsonObject ob = languages.at(i).toObject();

        mLanguageList.insert(ob["value"].toString(), ob["text"].toString());
    }
}

QMap<QString, QString> AppManager::getLanguageList() const
{
    return mLanguageList;
}

void AppManager::loadThemeList()
{
    QByteArray themesJson = FileUtil::readStringFromFile(":/static/themes.json").toUtf8();
    QJsonArray themes = QJsonDocument::fromJson(themesJson).array();

    for (int i = 0; i < themes.count(); ++i) {
        QJsonObject ob = themes.at(i).toObject();

        mThemeList.insert(ob["value"].toString(), ob["text"].toString());
    }
}

QMap<QString, QString> AppManager::getThemeList() const
{
    return mThemeList;
}

void AppManager::updateStylesheet()
{
    QString appThemePath = QString(":/static/themes/%1/style").arg(mSettingManager->getThemeName());
    mStyleValues = new QSettings(QString("%1/values.ini").arg(appThemePath), QSettings::IniFormat);

    mStylesheetFileContent = FileUtil::readStringFromFile(QString("%1/style.qss").arg(appThemePath));

    // set values example: @globalBackground => #212f3c
    for (const QString &key : mStyleValues->allKeys()) {
        mStylesheetFileContent.replace(key, mStyleValues->value(key).toString());
    }

    // apply font size offset
    int fontSizeOffset = mSettingManager->getFontSizeOffset();
    if (fontSizeOffset != 0) {
        QRegularExpression re("font-size:\\s*(\\d+)pt");
        QRegularExpressionMatchIterator it = re.globalMatch(mStylesheetFileContent);
        QList<QPair<int, int>> replacements; // position, original size
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            replacements.prepend({ match.capturedStart(1), match.captured(1).toInt() });
        }
        for (const QPair<int, int> &r : replacements) {
            int newSize = qMax(6, r.second + fontSizeOffset);
            mStylesheetFileContent.replace(r.first, QString::number(r.second).length(),
                                           QString::number(newSize));
        }
    }

    qApp->setStyleSheet(mStylesheetFileContent);

    emit SignalMapper::ins() -> sigChangedAppTheme();
}

QString AppManager::getStylesheetFileContent() const
{
    return mStylesheetFileContent;
}
