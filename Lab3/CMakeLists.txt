cmake_minimum_required(VERSION 3.10)

project(Timer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -D_UNIX")
endif()

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32")
endif()

set(SOURCES
    src/Timer.cpp  
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(UNIX)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

