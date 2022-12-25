project(yabba)
cmake_minimum_required(VERSION 3.5.0)

include(FindPkgConfig)

find_package(
    Qt5
    REQUIRED
        WebSockets
        Widgets
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(MOCUIC_DIR ${CMAKE_CURRENT_BINARY_DIR}/.mocuic)
set(YABBA_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
