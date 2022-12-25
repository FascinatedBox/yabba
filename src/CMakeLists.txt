qt5_add_resources(
    RCC_SOURCES
    resources.qrc
)

add_executable(
    yabba
        main.cpp
        ybotconnection.cpp
        ymainwindow.cpp
        ycontroller.cpp
        yinfodialog.cpp
        ysettings.cpp
        ywebsock.cpp
        ${RCC_SOURCES}
)

set_target_properties(
    yabba
    PROPERTIES
        AUTOGEN_BUILD_DIR
            ${MOCUIC_DIR}
        RUNTIME_OUTPUT_DIRECTORY
            ${YABBA_BINARY_DIR}
)

target_link_libraries(
    yabba
        Qt5::WebSockets
        Qt5::Widgets
)
