project(yabba)
cmake_minimum_required(VERSION 3.5.0)

find_package(
    Qt5
    COMPONENTS
        WebSockets
        Widgets
    REQUIRED
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(MOCUIC_DIR ${CMAKE_CURRENT_BINARY_DIR}/.mocuic)

qt5_add_resources(
    RCC_SOURCES
        src/resources.qrc
)

add_executable(
    yabba
        src/main.cpp
        src/ybotconnection.cpp
        src/ycontroller.cpp
        src/yinfodialog.cpp
        src/ymainwindow.cpp
        src/ysettings.cpp
        src/ywebsock.cpp
        ${RCC_SOURCES}
)

set_target_properties(
    yabba
    PROPERTIES
        AUTOGEN_BUILD_DIR
            ${MOCUIC_DIR}
)

target_link_libraries(
    yabba
        Qt5::WebSockets
        Qt5::Widgets
)
