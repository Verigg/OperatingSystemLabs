cmake_minimum_required(VERSION 3.10)

project(BackgroundProcessExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    background_process.h
    background_process.cpp
    main.cpp
)

add_executable(BackgroundProcessExample ${SOURCES})

if(WIN32)
    target_link_libraries(BackgroundProcessExample PRIVATE kernel32.lib)
else()
    target_compile_options(BackgroundProcessExample PRIVATE -Wall -Wextra)
    target_link_libraries(BackgroundProcessExample PRIVATE pthread)
endif()

