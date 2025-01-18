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

include(FetchContent)

FetchContent_Declare(
    SQLite3
    GIT_REPOSITORY https://github.com/sqlite/sqlite.git
    GIT_TAG        version-3.42.0 
)
FetchContent_MakeAvailable(SQLite3)

add_executable(Server ${SERVER_SOURCES})

if (WIN32)
    target_link_libraries(Server Ws2_32 sqlite3)
else()
    target_link_libraries(Server SQLite3)
endif()

add_executable(Emulator ${EMULATOR_SOURCES})
