#include "models/cleaner_model.h"
#include "models/process_model.h"
#include "models/service_model.h"
#include "models/startup_app_model.h"
#include "settings.h"
#include "tray.h"

#include "Info/cpu_info.h"
#include "Info/disk_info.h"
#include "Info/memory_info.h"
#include "Info/network_info.h"
#include "Info/system_info.h"
#include "Utils/format.h"
#include "Utils/hosts.h"

#include <QApplication>
#include <QFileInfo>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>
#include <QTranslator>

#include <KDBusService>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    app.setApplicationName(QStringLiteral(STACER_DESKTOP_NAME));
    app.setApplicationDisplayName(QStringLiteral("Stacer"));
    app.setApplicationVersion(QStringLiteral("1.7.0"));
    app.setDesktopFileName(QStringLiteral(STACER_DESKTOP_NAME));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("stacer")));

    const QString desktopStylePath = QLibraryInfo::path(QLibraryInfo::QmlImportsPath) + QStringLiteral("/org/kde/desktop");
    if (QFileInfo::exists(desktopStylePath)) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    } else {
        QQuickStyle::setStyle(QStringLiteral("Basic"));
    }

    QTranslator translator;
    const QString language = QLocale::system().name();
    const QStringList translationCandidates = {
        QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("translations/stacer_%1.qm").arg(language)),
        QCoreApplication::applicationDirPath() + QStringLiteral("/translations/stacer_%1.qm").arg(language),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../translations/stacer_%1.qm").arg(language),
    };
    for (const QString &candidate : translationCandidates) {
        if (!candidate.isEmpty() && translator.load(candidate)) {
            QCoreApplication::installTranslator(&translator);
            break;
        }
    }

    CpuInfo cpuInfo;
    MemoryInfo memoryInfo;
    NetworkInfo networkInfo;
    DiskInfo diskInfo;
    SystemInfo systemInfo;
    Format format;
    Settings settings;
    Tray tray;
    Hosts hosts;
    CleanerModel cleanerModel;
    ProcessModel processModel;
    ServiceModel serviceModel;
    StartupAppModel startupAppModel;

    qmlRegisterSingletonInstance("Stacer", 1, 0, "Cpu", &cpuInfo);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Memory", &memoryInfo);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Network", &networkInfo);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Disks", &diskInfo);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "SystemInformation", &systemInfo);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Format", &format);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Settings", &settings);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Tray", &tray);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Hosts", &hosts);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Cleaner", &cleanerModel);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Processes", &processModel);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "Services", &serviceModel);
    qmlRegisterSingletonInstance("Stacer", 1, 0, "StartupApps", &startupAppModel);

    KDBusService dbusService(KDBusService::Unique);

    QQmlApplicationEngine engine;

    QObject::connect(&dbusService, &KDBusService::activateRequested, &engine, [&engine](const QStringList &, const QString &) {
        const QList<QObject *> rootObjects = engine.rootObjects();
        if (rootObjects.isEmpty()) {
            return;
        }
        if (auto *window = qobject_cast<QQuickWindow *>(rootObjects.constFirst())) {
            window->show();
            window->raise();
            window->requestActivate();
        }
    });

    engine.loadFromModule("Stacer", "Main");
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    QTimer updateTimer;
    updateTimer.setInterval(1000);
    QObject::connect(&updateTimer, &QTimer::timeout, &app, [&] {
        cpuInfo.update();
        memoryInfo.update();
        networkInfo.update();
        diskInfo.update();
        systemInfo.update();
        processModel.update();
    });
    updateTimer.start();

    QObject::connect(&tray, &Tray::quitRequested, &app, &QCoreApplication::quit);
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &settings, &Settings::sync);

    return app.exec();
}
