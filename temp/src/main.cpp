#include <QApplication>
#include "MainWindow.h"
#include <iostream>

int main(int argc, char *argv[]) {
    std::cout << "Application started" << std::endl;
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}

