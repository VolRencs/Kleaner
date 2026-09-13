cmake_minimum_required(VERSION 3.16)
project(stacer)

include_directories(
  "${PROJECT_ROOT}/stacer-core"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/Managers"
  "${CMAKE_CURRENT_SOURCE_DIR}/Pages/Dashboard"
  "${CMAKE_CURRENT_SOURCE_DIR}/Pages/Processes"
  "${CMAKE_CURRENT_SOURCE_DIR}/Pages/Resources"
  "${CMAKE_CURRENT_SOURCE_DIR}/Pages/Services"
  "${CMAKE_CURRENT_SOURCE_DIR}/Pages/Settings"
  "${CMAKE_CURRENT_SOURCE_DIR}/Pages/StartupApps"
  "${CMAKE_CURRENT_SOURCE_DIR}/Pages/SystemCleaner"
  "${CMAKE_CURRENT_SOURCE_DIR}/Pages/Uninstaller"
)

# Sources
file(GLOB_RECURSE ${PROJECT_NAME}_srcs "${CMAKE_CURRENT_SOURCE_DIR}/**.cpp")
file(GLOB ${PROJECT_NAME}_translations "${PROJECT_ROOT}/translations/*.ts")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME}
  ${${PROJECT_NAME}_srcs}
  "${CMAKE_CURRENT_SOURCE_DIR}/static.qrc"
)

set_source_files_properties(
  ${${PROJECT_NAME}_translations}
  PROPERTIES
  OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations"
)

# Translations
find_package(Qt${QT_VERSION_MAJOR} REQUIRED LinguistTools)
if("${QT_VERSION_MAJOR}" STREQUAL "5")
  qt5_add_translation(QM_FILES ${${PROJECT_NAME}_translations} ${${PROJECT_NAME}_srcs})
else()
  qt6_add_translations(stacer TS_FILES ${${PROJECT_NAME}_translations} QM_FILES_OUTPUT_VARIABLE QM_FILES)
endif()
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${QM_FILES}")

target_link_libraries(${PROJECT_NAME} stacer-core)
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Svg Qt${QT_VERSION_MAJOR}::Concurrent)

# Ensure C++ standard library is linked when the linker doesn't add it automatically
target_link_libraries(${PROJECT_NAME} stdc++)
target_link_libraries(${PROJECT_NAME} m)

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
)

install(
  FILES ${QM_FILES}
  DESTINATION share/stacer/translations
)
