cmake_minimum_required(VERSION 3.15)
project(displaylink-direct-qpa VERSION 1.0.0 LANGUAGES CXX)

option(BUILD_PLUGIN "Build the Qt platform plugin" ON)
option(BUILD_TESTS "Build unit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(src/core)

if (BUILD_PLUGIN)
    find_package(dlsdk 2.0.0 REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        FbSupportPrivate
        InputSupportPrivate
    )

    # Install into the selected Qt installation's platform plugin directory by default
    include(GNUInstallDirs)
    if (DEFINED QT6_INSTALL_PREFIX AND DEFINED QT6_INSTALL_PLUGINS)
        set(QDLSDK_PLUGIN_INSTALL_DIR_DEFAULT "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_PLUGINS}/platforms")
    else()
        set(QDLSDK_PLUGIN_INSTALL_DIR_DEFAULT "${CMAKE_INSTALL_LIBDIR}/qt6/plugins/platforms")
    endif()

    set(QDLSDK_PLUGIN_INSTALL_DIR "${QDLSDK_PLUGIN_INSTALL_DIR_DEFAULT}" CACHE PATH
        "Install directory for the qdlsdk Qt platform plugin"
    )

    add_subdirectory(src/plugin)
endif()

if (BUILD_TESTS)
    # Prefer include(CTest) over enable_testing()
    include(CTest)
    include(FetchContent)

    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723 # v1.17.0
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests/core)
    if (BUILD_PLUGIN)
        add_subdirectory(tests/plugin)
    endif()
endif()
