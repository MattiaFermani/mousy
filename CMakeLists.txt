cmake_minimum_required(VERSION 3.16)

# Read base version from VERSION file
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" BASE_VERSION LIMIT_COUNT 1)
if(NOT BASE_VERSION)
    set(BASE_VERSION "1.0.0")
endif()
string(STRIP "${BASE_VERSION}" BASE_VERSION)

# Extract MAJOR, MINOR, PATCH from BASE_VERSION
if(BASE_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)")
    set(MOUSY_VERSION_MAJOR "${CMAKE_MATCH_1}")
    set(MOUSY_VERSION_MINOR "${CMAKE_MATCH_2}")
    set(MOUSY_VERSION_PATCH "${CMAKE_MATCH_3}")
else()
    set(MOUSY_VERSION_MAJOR "1")
    set(MOUSY_VERSION_MINOR "0")
    set(MOUSY_VERSION_PATCH "0")
endif()

project(Mousy
    VERSION "${MOUSY_VERSION_MAJOR}.${MOUSY_VERSION_MINOR}.${MOUSY_VERSION_PATCH}"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Automatic Git version extraction
set(MOUSY_GIT_COMMIT "unknown")
set(MOUSY_GIT_BRANCH "master")
set(MOUSY_GIT_TAG "v${BASE_VERSION}")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE MOUSY_GIT_COMMIT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE MOUSY_GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --match "v*" --always --dirty
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE MOUSY_GIT_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
    endif()
endif()

# Parse SemVer if git tag exists
if(MOUSY_GIT_TAG MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)(.*)")
    set(MOUSY_VERSION_MAJOR "${CMAKE_MATCH_1}")
    set(MOUSY_VERSION_MINOR "${CMAKE_MATCH_2}")
    set(MOUSY_VERSION_PATCH "${CMAKE_MATCH_3}")
    set(MOUSY_VERSION_EXTRA "${CMAKE_MATCH_4}")
    set(MOUSY_VERSION_STRING "v${MOUSY_VERSION_MAJOR}.${MOUSY_VERSION_MINOR}.${MOUSY_VERSION_PATCH}${MOUSY_VERSION_EXTRA}")
else()
    set(MOUSY_VERSION_STRING "v${MOUSY_VERSION_MAJOR}.${MOUSY_VERSION_MINOR}.${MOUSY_VERSION_PATCH}")
endif()

string(TIMESTAMP MOUSY_BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)

# Generate Version.h
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/src/Version.h"
    @ONLY
)

add_executable(Mousy
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/MouseViewWidget.cpp
    src/MouseViewWidget.h
    src/HeatmapWidget.cpp
    src/HeatmapWidget.h
    src/AimTrainerWidget.cpp
    src/AimTrainerWidget.h
    src/MacroWidget.cpp
    src/MacroWidget.h
    "${CMAKE_CURRENT_BINARY_DIR}/src/Version.h"
)

target_include_directories(Mousy PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}/src"
)

target_link_libraries(Mousy PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
