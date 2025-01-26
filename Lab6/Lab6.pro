TEMPLATE = app
TARGET = TemperatureClientApp
CONFIG += c++17 console qt quickwidgets
QT += core gui widgets network

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/MainWindow.h

FORMS += \
    src/MainWindow.ui

win32 {
    QT6_DIR = C:/CppLibs/Qt/6.8.1/mingw_64/lib/cmake/Qt6
    QWT_INCLUDE_DIR = C:/CppLibs/Qwt-6.3.0/src
    QWT_LIB_DIR = C:/CppLibs/Qwt-6.3.0/lib
    QWT_LIBRARY = C:/CppLibs/Qwt-6.3.0/lib/libqwt.a

    INCLUDEPATH += $$QWT_INCLUDE_DIR
    LIBS += -L$$QWT_LIB_DIR -lqwt
    DEFINES += WIN32
}

unix{
	INCLUDEPATH += /usr/include/qwt
	LIBS += -L/usr/lib -lqwt-qt5
}


CONFIG += qmake_qt_autogen
