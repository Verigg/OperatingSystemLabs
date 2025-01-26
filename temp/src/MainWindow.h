#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void fetchCurrentTemperature();
    void fetchTemperatureStats();
    void onStatsReply(QNetworkReply *reply);
    void onCurrentReply(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    QwtPlotCurve *temperatureCurve;

    void updateGraph(const QVector<double> &timestamps, const QVector<double> &temperatures);
};

#endif // MAINWINDOW_H
