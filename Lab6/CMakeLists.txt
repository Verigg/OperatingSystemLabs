cmake_minimum_required(VERSION 3.16)

project(TemperatureClientApp LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(Qt6_DIR "C:/CppLibs/Qt/6.8.1/mingw_64/lib/cmake/Qt6")
    set(QWT_INCLUDE_DIR "C:/CppLibs/Qwt-6.3.0/src")
    set(QWT_LIB_DIR "C:/CppLibs/Qwt-6.3.0/lib/")
    set(QWT_LIBRARY "C:/CppLibs/Qwt-6.3.0/lib/libqwt.a")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

include_directories(${QWT_INCLUDE_DIR})
link_directories(${QWT_LIB_DIR})

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
)

set(HEADERS
    src/MainWindow.h
)

set(UI
    src/MainWindow.ui
)

qt_add_executable(TemperatureClientApp
    ${SOURCES}
    ${HEADERS}
    ${UI}
)

target_link_libraries(TemperatureClientApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

target_include_directories(TemperatureClientApp PRIVATE ${QWT_INCLUDE_DIR})
target_link_libraries(TemperatureClientApp PRIVATE ${QWT_LIBRARY})

include_directories(${Qt6_INCLUDE_DIR} ${QWT_INCLUDE_DIR})

if(WIN32)
    add_definitions(-DWIN32)
    add_compile_options(/EHsc)
endif()
