cmake_minimum_required(VERSION 3.14)

project(AuthServer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_executable(AuthServer
    main.cpp  packetreader.cpp  packetwriter.cpp  socketserver.cpp field.cpp
    packetreader.h  packetwriter.h  socketserver.h field.h
    client.h client.cpp
)

target_link_libraries(AuthServer Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)

install(TARGETS AuthServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
