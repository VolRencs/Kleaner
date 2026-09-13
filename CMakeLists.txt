cmake_minimum_required(VERSION 3.21)

project(kleaner VERSION 1.7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)

# KDE CMake infrastructure
find_package(ECM 6.30 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(CTest)

# Single source of truth for the application identity.
# Changing these two variables renames the application everywhere.
set(KLEANER_APP_ID "fr.quentium.kleaner" CACHE STRING "Application/DBus/policy ID")
set(KLEANER_DESKTOP_NAME "kleaner" CACHE STRING "Desktop entry base name")
set(KLEANER_HELPER_ID "${KLEANER_APP_ID}")

add_compile_definitions(
  KLEANER_APP_ID="${KLEANER_APP_ID}"
  KLEANER_DESKTOP_NAME="${KLEANER_DESKTOP_NAME}"
  KLEANER_HELPER_ID="${KLEANER_HELPER_ID}"
)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Network DBus)
find_package(KF6 6.30 REQUIRED COMPONENTS CoreAddons)

add_subdirectory(kleaner-core)
add_subdirectory(kleaner)

string(TIMESTAMP TODAY_ISO_8601 "%Y-%m-%d" UTC)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/desktop/kleaner.metainfo.xml.in"
  "${CMAKE_CURRENT_BINARY_DIR}/desktop/${KLEANER_APP_ID}.metainfo.xml"
  IMMEDIATE @ONLY
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/desktop/${KLEANER_APP_ID}.metainfo.xml"
  DESTINATION ${KDE_INSTALL_METAINFODIR}
)

install(
  FILES "${CMAKE_CURRENT_SOURCE_DIR}/desktop/kleaner.desktop"
  DESTINATION ${KDE_INSTALL_APPDIR}
  RENAME "${KLEANER_DESKTOP_NAME}.desktop"
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/icons/hicolor"
  DESTINATION ${KDE_INSTALL_ICONDIR}
)
