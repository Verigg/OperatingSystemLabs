cmake_minimum_required(VERSION 3.16)
project(TemperatureClientApp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Установите стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Установка путей для Qt
if(WIN32)
    set(Qt6_DIR "C:/CppLibs/Qt/6.8.1/mingw_64/lib/cmake/Qt6")
    set(QWT_INCLUDE_DIR "C:/CppLibs/qwt-6.3.0/src")
    set(QWT_LIBRARY "C:/CppLibs/qwt-6.3.0/lib/libqwt.a")  # Убедитесь, что это правильно
    list(APPEND CMAKE_PREFIX_PATH "C:/CppLibs/Qt/6.8.1/mingw_64")
elseif(UNIX)
    set(Qt6_DIR "/usr/lib/cmake/Qt6")
    set(QWT_INCLUDE_DIR "/usr/local/include/qwt")
    set(QWT_LIBRARY "/usr/local/lib/libqwt.a")  # Для Unix нужно использовать .a или .so
    list(APPEND CMAKE_PREFIX_PATH "/usr/lib/qt6")
endif()

# Поиск Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt6_wrap_ui(HEADERS src/MainWindow.ui)


# Исходные файлы
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
)

set(UI
    src/MainWindow.ui
)

set(HEADERS
    src/MainWindow.h
)

# Создание исполняемого файла
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI})

# Включение заголовочных файлов
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QWT_INCLUDE_DIR}
    ${Qt6_INCLUDE_DIRS}
)

# Линковка с библиотеками
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Network
    ${QWT_LIBRARY}
)

# Платформенные параметры
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
endif()
