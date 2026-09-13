cmake_minimum_required(VERSION 3.21)

project(stacer VERSION 1.7.0 LANGUAGES CXX)

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

# Single source of truth for the application identity.
# The fork will be renamed later; changing these two variables is enough.
set(STACER_APP_ID "fr.quentium.stacer" CACHE STRING "Application/DBus/policy ID")
set(STACER_DESKTOP_NAME "stacer" CACHE STRING "Desktop entry base name")
set(STACER_HELPER_ID "${STACER_APP_ID}")

add_compile_definitions(
  STACER_APP_ID="${STACER_APP_ID}"
  STACER_DESKTOP_NAME="${STACER_DESKTOP_NAME}"
  STACER_HELPER_ID="${STACER_HELPER_ID}"
)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Network DBus)
find_package(KF6 6.30 REQUIRED COMPONENTS CoreAddons)

add_subdirectory(stacer-core)
add_subdirectory(stacer)

string(TIMESTAMP TODAY_ISO_8601 "%Y-%m-%d" UTC)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/desktop/stacer.metainfo.xml.in"
  "${CMAKE_CURRENT_BINARY_DIR}/desktop/${STACER_APP_ID}.metainfo.xml"
  IMMEDIATE @ONLY
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/desktop/${STACER_APP_ID}.metainfo.xml"
  DESTINATION ${KDE_INSTALL_METAINFODIR}
)

install(
  FILES "${CMAKE_CURRENT_SOURCE_DIR}/desktop/stacer.desktop"
  DESTINATION ${KDE_INSTALL_APPDIR}
  RENAME "${STACER_DESKTOP_NAME}.desktop"
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/icons/hicolor"
  DESTINATION ${KDE_INSTALL_ICONDIR}
)
