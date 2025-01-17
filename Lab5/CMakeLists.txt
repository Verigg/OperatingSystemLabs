cmake_minimum_required(VERSION 3.10)

project(TemperatureMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

include_directories(${INCLUDE_DIR})

set(SERIAL_PORT ${SRC_DIR}/SerialPort.cpp)

set(SERVER_SOURCES
    ${SRC_DIR}/Server.cpp
    ${SERIAL_PORT}
)

set(EMULATOR_SOURCES
    ${SRC_DIR}/Emulator.cpp
    ${SERIAL_PORT}
)

add_executable(Server ${SERVER_SOURCES})
if (WIN32)
    target_link_libraries(Server Ws2_32)
endif()

add_executable(Emulator ${EMULATOR_SOURCES})
