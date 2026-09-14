# SPDX-FileCopyrightText: 2026 VolRen
# SPDX-License-Identifier: GPL-3.0-only

find_package(Qt6 6.11 REQUIRED COMPONENTS Quick QuickControls2 Svg Widgets Concurrent LinguistTools)
find_package(KF6 6.30 REQUIRED COMPONENTS Kirigami Config DBusAddons StatusNotifierItem Auth)

qt_add_executable(kleaner
    main.cpp
    settings.cpp
    settings.h
    tray.cpp
    tray.h
    models/process_model.cpp
    models/process_model.h
    models/service_model.cpp
    models/service_model.h
    models/startup_app_model.cpp
    models/startup_app_model.h
    models/cleaner_model.cpp
    models/cleaner_model.h
)

target_link_libraries(kleaner
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Svg
        Qt6::Widgets
        Qt6::Concurrent
        KF6::Kirigami
        KF6::ConfigCore
        KF6::DBusAddons
        KF6::StatusNotifierItem
        kleaner-core
)

set_source_files_properties(qml/Design.qml qml/History.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(kleaner
    URI Kleaner
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Design.qml
        qml/History.qml
        qml/components/AppButton.qml
        qml/components/AppCard.qml
        qml/components/AppCheckBox.qml
        qml/components/AppComboBox.qml
        qml/components/AppDialog.qml
        qml/components/AppDialogFooter.qml
        qml/components/AppScrollBar.qml
        qml/components/AppScrollView.qml
        qml/components/AppSearchField.qml
        qml/components/AppSpinner.qml
        qml/components/AppSwitch.qml
        qml/components/AppTextField.qml
        qml/components/Badge.qml
        qml/components/ChartCard.qml
        qml/components/Gauge.qml
        qml/components/InfoRow.qml
        qml/components/NavItem.qml
        qml/components/PageHeader.qml
        qml/components/SettingRow.qml
        qml/components/SortHeader.qml
        qml/pages/DashboardPage.qml
        qml/pages/ResourcesPage.qml
        qml/pages/ProcessesPage.qml
        qml/pages/ServicesPage.qml
        qml/pages/StartupAppsPage.qml
        qml/pages/CleanerPage.qml
        qml/pages/HostsPage.qml
        qml/pages/SettingsPage.qml
        qml/pages/AboutKleanerPage.qml
)

qt_add_resources(kleaner "kleaner-resources"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    FILES
        resources/kleaner-circle.svg
        resources/kleaner-k.svg
)

file(GLOB KLEANER_TS_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/translations/*.ts")

qt_add_translations(kleaner
    TARGETS kleaner
    TS_FILES ${KLEANER_TS_FILES}
    LUPDATE_OPTIONS -no-obsolete
    QM_FILES_OUTPUT_VARIABLE KLEANER_QM_FILES
)

install(
    FILES ${KLEANER_QM_FILES}
    DESTINATION ${KDE_INSTALL_DATADIR}/${KLEANER_DESKTOP_NAME}/translations
)

# Privileged helper (KAuth / polkit)
qt_add_executable(kleaner-kauth-helper helper/main.cpp)

target_link_libraries(kleaner-kauth-helper PRIVATE KF6::AuthCore)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/helper/kleaner.actions.in"
    "${CMAKE_CURRENT_BINARY_DIR}/org.volren.kleaner.actions"
    @ONLY
)

kauth_install_helper_files(kleaner-kauth-helper ${KLEANER_HELPER_ID} root)
kauth_install_actions(${KLEANER_HELPER_ID} "${CMAKE_CURRENT_BINARY_DIR}/org.volren.kleaner.actions")

install(TARGETS kleaner-kauth-helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

install(TARGETS kleaner ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
