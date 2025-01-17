cmake_minimum_required(VERSION 3.10)
project(TemperatureMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(SOURCES
    ${SRC_DIR}/Emulator.cpp
    ${SRC_DIR}/httplib.h
    ${SRC_DIR}/Server.cpp
)

find_package(SQLite3 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include ${SQLite3_INCLUDE_DIRS})

add_executable(TemperatureMonitor ${SOURCES})

target_link_libraries(TemperatureMonitor ${SQLite3_LIBRARIES})

if(WIN32)
    target_compile_definitions(TemperatureMonitor PRIVATE _WIN32_WINNT=0x0601)
    target_link_libraries(TemperatureMonitor ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(TemperatureMonitor Threads::Threads)
endif()
