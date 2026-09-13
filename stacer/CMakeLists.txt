find_package(Qt6 6.11 REQUIRED COMPONENTS Quick QuickControls2 Svg Widgets LinguistTools)
find_package(KF6 6.30 REQUIRED COMPONENTS Kirigami QuickCharts Config DBusAddons StatusNotifierItem)

qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(stacer
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

target_include_directories(stacer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(stacer
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Svg
        Qt6::Widgets
        KF6::Kirigami
        KF6::QuickCharts
        KF6::ConfigCore
        KF6::DBusAddons
        KF6::StatusNotifierItem
        stacer-core
)

qt_add_qml_module(stacer
    URI Stacer
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/Donut.qml
        qml/components/ChartCard.qml
        qml/pages/DashboardPage.qml
        qml/pages/ResourcesPage.qml
        qml/pages/ProcessesPage.qml
        qml/pages/ServicesPage.qml
        qml/pages/StartupAppsPage.qml
        qml/pages/CleanerPage.qml
        qml/pages/HostsPage.qml
        qml/pages/SettingsPage.qml
        qml/pages/AboutStacerPage.qml
)

file(GLOB STACER_TS_FILES "${CMAKE_SOURCE_DIR}/translations/*.ts")

qt_add_translations(stacer
    TS_FILES ${STACER_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE STACER_QM_FILES
)

install(
    FILES ${STACER_QM_FILES}
    DESTINATION ${KDE_INSTALL_DATADIR}/${STACER_DESKTOP_NAME}/translations
)

# Privileged helper (KAuth / polkit)
qt_add_executable(stacer-kauth-helper helper/main.cpp)

target_link_libraries(stacer-kauth-helper PRIVATE Qt6::Core KF6::AuthCore)

find_package(KF6Auth REQUIRED)
include("${KF6Auth_DIR}/KF6AuthMacros.cmake")

set(STACER_HELPER_EXEC "${KAUTH_HELPER_INSTALL_ABSOLUTE_DIR}/stacer-kauth-helper")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/helper/fr.quentium.stacer.actions.in"
    "${CMAKE_CURRENT_BINARY_DIR}/fr.quentium.stacer.actions"
    @ONLY
)

kauth_install_helper_files(stacer-kauth-helper ${STACER_HELPER_ID} root)
kauth_install_actions(${STACER_HELPER_ID} "${CMAKE_CURRENT_BINARY_DIR}/fr.quentium.stacer.actions")

install(TARGETS stacer-kauth-helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

install(TARGETS stacer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
