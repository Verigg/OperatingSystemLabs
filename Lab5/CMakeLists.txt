cmake_minimum_required(VERSION 3.10)

project(TemperatureMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR src)
set(INCLUDE_DIR include)

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

add_library(SerialPort ${SERIAL_PORT})
target_include_directories(SerialPort PRIVATE ${INCLUDE_DIR})

add_executable(Server ${SERVER_SOURCES})
target_link_libraries(Server PRIVATE SerialPort SQLite3)

if (WIN32)
    target_link_libraries(Server PRIVATE Ws2_32)
    target_compile_definitions(Server PRIVATE WINDOWS)
endif()

add_executable(Emulator ${EMULATOR_SOURCES})
target_link_libraries(Emulator PRIVATE SerialPort)


if (WIN32)
    target_link_libraries(Server Ws2_32 SQLite3)
else()
    target_link_libraries(Server SQLite3)
endif()

add_executable(Emulator ${EMULATOR_SOURCES})
