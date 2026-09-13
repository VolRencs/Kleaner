find_package(Qt6 6.11 REQUIRED COMPONENTS Concurrent)
find_package(KF6 6.30 REQUIRED COMPONENTS Auth KIO)

add_library(stacer-core STATIC
    Info/cpu_info.cpp
    Info/disk_info.cpp
    Info/memory_info.cpp
    Info/network_info.cpp
    Info/process_info.cpp
    Info/system_info.cpp
    Services/service_backend.cpp
    Startup/startup_apps.cpp
    Cleaner/cleaner.cpp
    Utils/format.cpp
    Utils/helpers.cpp
    Utils/hosts.cpp
    Utils/procfs.cpp
)

target_include_directories(stacer-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(stacer-core
    PUBLIC
        Qt6::Core
        Qt6::DBus
        Qt6::Concurrent
        KF6::CoreAddons
        KF6::AuthCore
        KF6::KIOCore
)

if(BUILD_TESTING)
    find_package(Qt6 6.11 REQUIRED COMPONENTS Test)

    add_executable(stacer-core-tests tests/core_test.cpp)
    target_link_libraries(stacer-core-tests PRIVATE stacer-core Qt6::Test)
    add_test(NAME stacer-core-tests COMMAND stacer-core-tests)
endif()
