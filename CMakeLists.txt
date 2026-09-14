# SPDX-FileCopyrightText: 2026 VolRen
# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.22)

project(kleaner
    VERSION 1.2.1
    DESCRIPTION "Linux System Optimizer and Monitoring"
    HOMEPAGE_URL "https://volrencs.github.io/Kleaner/"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# KDE CMake infrastructure
find_package(ECM 6.30 REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEClangFormat)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Quick QuickControls2 DBus)
find_package(KF6 6.30 REQUIRED COMPONENTS CoreAddons)

qt_standard_project_setup(REQUIRES 6.11)

# Single source of truth for the application identity.
# Changing these variables renames the application everywhere.
set(KLEANER_APP_ID "org.volren.kleaner" CACHE STRING "Application/DBus/policy ID")
set(KLEANER_DESKTOP_NAME "${KLEANER_APP_ID}" CACHE STRING "Desktop entry base name")
set(KLEANER_HELPER_ID "${KLEANER_APP_ID}")

add_compile_definitions(
  KLEANER_APP_ID="${KLEANER_APP_ID}"
  KLEANER_DESKTOP_NAME="${KLEANER_DESKTOP_NAME}"
  KLEANER_HELPER_ID="${KLEANER_HELPER_ID}"
  KLEANER_VERSION="${PROJECT_VERSION}"
)

add_subdirectory(kleaner-core)
add_subdirectory(kleaner)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/desktop/kleaner.metainfo.xml.in"
  "${CMAKE_CURRENT_BINARY_DIR}/desktop/${KLEANER_APP_ID}.metainfo.xml"
  @ONLY
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/desktop/${KLEANER_APP_ID}.metainfo.xml"
  DESTINATION ${KDE_INSTALL_METAINFODIR}
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/desktop/kleaner.desktop.in"
  "${CMAKE_CURRENT_BINARY_DIR}/desktop/${KLEANER_APP_ID}.desktop"
  @ONLY
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/desktop/${KLEANER_APP_ID}.desktop"
  DESTINATION ${KDE_INSTALL_APPDIR}
)

# Session D-Bus activation, required by DBusActivatable=true in the desktop entry
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/desktop/kleaner.service.in"
  "${CMAKE_CURRENT_BINARY_DIR}/desktop/${KLEANER_APP_ID}.service"
  @ONLY
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/desktop/${KLEANER_APP_ID}.service"
  DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/icons/hicolor"
  DESTINATION ${KDE_INSTALL_ICONDIR}
)

install(
  FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
  DESTINATION "${KDE_INSTALL_DATADIR}/doc/${KLEANER_APP_ID}"
)

file(GLOB_RECURSE KLEANER_CLANG_FORMAT_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/kleaner/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/kleaner/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/kleaner-core/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/kleaner-core/*.h"
)

kde_clang_format(${KLEANER_CLANG_FORMAT_FILES})
