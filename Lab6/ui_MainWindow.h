/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "qwt_plot.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QLabel *label;
    QLabel *currentTemperatureLabel;
    QLabel *label_2;
    QLabel *label_3;
    QDateTimeEdit *startDateTimeEdit;
    QDateTimeEdit *endDateTimeEdit;
    QPushButton *updateButton;
    QwtPlot *qwtPlot;
    QLabel *statusLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(657, 402);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setGeometry(QRect(20, 10, 121, 16));
        currentTemperatureLabel = new QLabel(centralwidget);
        currentTemperatureLabel->setObjectName("currentTemperatureLabel");
        currentTemperatureLabel->setGeometry(QRect(150, 10, 49, 16));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(20, 50, 91, 16));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(330, 50, 91, 16));
        startDateTimeEdit = new QDateTimeEdit(centralwidget);
        startDateTimeEdit->setObjectName("startDateTimeEdit");
        startDateTimeEdit->setGeometry(QRect(120, 50, 194, 22));
        endDateTimeEdit = new QDateTimeEdit(centralwidget);
        endDateTimeEdit->setObjectName("endDateTimeEdit");
        endDateTimeEdit->setGeometry(QRect(430, 50, 194, 22));
        updateButton = new QPushButton(centralwidget);
        updateButton->setObjectName("updateButton");
        updateButton->setGeometry(QRect(20, 320, 141, 31));
        qwtPlot = new QwtPlot(centralwidget);
        qwtPlot->setObjectName("qwtPlot");
        qwtPlot->setGeometry(QRect(30, 90, 591, 200));
        statusLabel = new QLabel(centralwidget);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setGeometry(QRect(180, 330, 461, 16));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 657, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\320\242\320\265\320\272\321\203\321\211\320\260\321\217 \321\202\320\265\320\274\320\277\320\265\321\200\320\260\321\202\321\203\321\200\320\260:", nullptr));
        currentTemperatureLabel->setText(QCoreApplication::translate("MainWindow", "---- \302\260C", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "\320\235\320\260\321\207\320\260\320\273\320\276 \320\277\320\265\321\200\320\270\320\276\320\264\320\260:", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "\320\232\320\276\320\275\320\265\321\206 \320\277\320\265\321\200\320\270\320\276\320\264\320\260:", nullptr));
        updateButton->setText(QCoreApplication::translate("MainWindow", "\320\236\320\261\320\275\320\276\320\262\320\270\321\202\321\214 \320\263\321\200\320\260\321\204\320\270\320\272", nullptr));
        statusLabel->setText(QCoreApplication::translate("MainWindow", "\320\223\320\276\321\202\320\276\320\262\320\276", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
