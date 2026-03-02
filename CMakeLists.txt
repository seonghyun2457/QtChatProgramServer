cmake_minimum_required(VERSION 3.19)
project(QtChatProgramServer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

qt_add_executable(QtChatProgramServer
    main.cpp
    server.h server.cpp
    client.h client.cpp
    clientthread.h clientthread.cpp
)

target_link_libraries(QtChatProgramServer PRIVATE Qt::Core Qt6::Network)

include(GNUInstallDirs)

install(TARGETS QtChatProgramServer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
