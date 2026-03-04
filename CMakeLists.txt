cmake_minimum_required(VERSION 3.19)
project(QtChatProgramServer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network Widgets)

qt_standard_project_setup()

qt_add_executable(QtChatProgramServer
    WIN32 MACOSX_BUNDLE
    main.cpp
    server.h server.cpp
    client.h client.cpp
    clientthread.h clientthread.cpp
    serverwindow.h serverwindow.cpp serverwindow.ui
)

target_link_libraries(QtChatProgramServer PRIVATE Qt::Core Qt6::Network Qt6::Widgets)

include(GNUInstallDirs)

install(TARGETS QtChatProgramServer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QtChatProgramServer
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
