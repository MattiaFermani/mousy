cmake_minimum_required(VERSION 3.16)
project(Mousy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

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
)

target_link_libraries(Mousy PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
