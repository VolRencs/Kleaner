// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

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
#include <QPalette>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <QTranslator>

#include <utility>

#include <KDBusService>

namespace
{
void applyDarkPalette()
{
    const QColor window(0x0f, 0x11, 0x15);
    const QColor surface(0x16, 0x1a, 0x21);
    const QColor surfaceHover(0x1c, 0x21, 0x2b);
    const QColor elevated(0x1a, 0x1f, 0x28);
    const QColor border(0x23, 0x2a, 0x35);
    const QColor text(0xee, 0xf1, 0xf6);
    const QColor textFaint(0x5f, 0x6a, 0x7c);
    const QColor accent(0x3d, 0xae, 0xe9);
    const QColor accentDark(0x08, 0x13, 0x1a);
    const QColor highlightText(0xa7, 0x8b, 0xfa);

    QPalette palette;
    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, surface);
    palette.setColor(QPalette::AlternateBase, surfaceHover);
    palette.setColor(QPalette::ToolTipBase, elevated);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, surfaceHover);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::Light, border);
    palette.setColor(QPalette::Midlight, surfaceHover);
    palette.setColor(QPalette::Mid, border);
    palette.setColor(QPalette::Dark, window);
    palette.setColor(QPalette::Shadow, Qt::black);
    palette.setColor(QPalette::Highlight, accent);
    palette.setColor(QPalette::HighlightedText, accentDark);
    palette.setColor(QPalette::Link, accent);
    palette.setColor(QPalette::LinkVisited, highlightText);
    palette.setColor(QPalette::PlaceholderText, textFaint);
    palette.setColor(QPalette::Accent, accent);

    palette.setColor(QPalette::Disabled, QPalette::WindowText, textFaint);
    palette.setColor(QPalette::Disabled, QPalette::Text, textFaint);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, textFaint);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, border);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, textFaint);

    QApplication::setPalette(palette);
}
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // KDBusService derives its bus name as reversed organizationDomain + applicationName,
    // which must result in org.volren.kleaner to match the desktop id and the D-Bus
    // session service file used for activation from the application menu.
    app.setOrganizationDomain(QStringLiteral("volren.org"));
    app.setApplicationName(QStringLiteral("kleaner"));
    app.setApplicationDisplayName(QStringLiteral("Kleaner"));
    app.setApplicationVersion(QStringLiteral(KLEANER_VERSION));
    app.setDesktopFileName(QStringLiteral(KLEANER_DESKTOP_NAME));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral(KLEANER_DESKTOP_NAME), QIcon(QStringLiteral(":/kleaner-circle.svg"))));

    // Created before the models so that a duplicate launch quits immediately
    // instead of building the whole application state first.
    KDBusService dbusService(KDBusService::Unique);

    const QString desktopStylePath = QLibraryInfo::path(QLibraryInfo::QmlImportsPath) + QStringLiteral("/org/kde/desktop");
    if (QFileInfo::exists(desktopStylePath)) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    } else {
        QQuickStyle::setStyle(QStringLiteral("Basic"));
    }

    applyDarkPalette();

    Settings settings;

    QTranslator appTranslator;
    QTranslator qtTranslator;
    auto applyLanguage = [&settings, &appTranslator, &qtTranslator] {
        QCoreApplication::removeTranslator(&appTranslator);
        QCoreApplication::removeTranslator(&qtTranslator);

        const QString configured = settings.language();
        QStringList codes;
        if (configured.isEmpty()) {
            const QLocale system = QLocale::system();
            for (const QString &uiLanguage : system.uiLanguages()) {
                codes.append(uiLanguage);
                codes.append(uiLanguage.section(QLatin1Char('_'), 0, 0));
                codes.append(uiLanguage.section(QLatin1Char('-'), 0, 0));
            }
            codes.append(system.name());
            codes.append(system.name().section(QLatin1Char('_'), 0, 0));
            codes.removeAll(QString());
            codes.removeDuplicates();
        } else {
            codes.append(configured);
            QLocale::setDefault(QLocale(configured));
        }

        bool appLoaded = false;
        for (const QString &directory : Settings::translationDirectories()) {
            if (appLoaded) {
                break;
            }
            for (const QString &code : std::as_const(codes)) {
                const QString path = directory + QStringLiteral("/kleaner_%1.qm").arg(code);
                if (QFileInfo::exists(path) && appTranslator.load(path)) {
                    QCoreApplication::installTranslator(&appTranslator);
                    appLoaded = true;
                    break;
                }
            }
        }

        // Qt's own strings (dialogs, controls) and KDE framework messages.
        const QString qtDirectory = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
        for (const QString &code : std::as_const(codes)) {
            if (qtTranslator.load(QStringLiteral("qtbase_%1").arg(code), qtDirectory)) {
                QCoreApplication::installTranslator(&qtTranslator);
                break;
            }
        }
    };
    applyLanguage();

    CpuInfo cpuInfo;
    MemoryInfo memoryInfo;
    NetworkInfo networkInfo;
    DiskInfo diskInfo;
    SystemInfo systemInfo;
    Format format;
    Tray tray;
    Hosts hosts;
    CleanerModel cleanerModel;
    ProcessModel processModel;
    ServiceModel serviceModel;
    StartupAppModel startupAppModel;

    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Cpu", &cpuInfo);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Memory", &memoryInfo);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Network", &networkInfo);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Disks", &diskInfo);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "SystemInformation", &systemInfo);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Format", &format);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Settings", &settings);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Tray", &tray);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Hosts", &hosts);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Cleaner", &cleanerModel);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Processes", &processModel);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "Services", &serviceModel);
    qmlRegisterSingletonInstance("Kleaner", 1, 0, "StartupApps", &startupAppModel);

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

    engine.loadFromModule("Kleaner", "Main");
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    QObject::connect(&settings, &Settings::languageChanged, &engine, [&applyLanguage, &engine] {
        applyLanguage();
        engine.retranslate();
    });

    QTimer updateTimer;
    updateTimer.setInterval(1000);
    QObject::connect(&updateTimer, &QTimer::timeout, &app, [&] {
        cpuInfo.update();
        memoryInfo.update();
        networkInfo.update();
        diskInfo.update();
        systemInfo.update();
    });
    updateTimer.start();

    QObject::connect(&tray, &Tray::quitRequested, &app, &QCoreApplication::quit);
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &settings, &Settings::sync);

    return app.exec();
}
