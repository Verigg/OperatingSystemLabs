cmake_minimum_required(VERSION 3.10)

project(SerialPortTools)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

set(EMULATOR_SOURCES
    src/Emulator.cpp
)

set(LOGGER_SOURCES
    src/Logger.cpp
)

if (WIN32)
    set(PLATFORM_LIBS kernel32)
else()
    set(PLATFORM_LIBS pthread)
endif()

add_executable(emulator ${EMULATOR_SOURCES})
target_link_libraries(emulator ${PLATFORM_LIBS})

add_executable(logger ${LOGGER_SOURCES})
target_link_libraries(logger ${PLATFORM_LIBS})

install(TARGETS emulator logger
        RUNTIME DESTINATION bin)
