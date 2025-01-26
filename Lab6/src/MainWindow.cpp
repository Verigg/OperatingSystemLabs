#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    networkManager(new QNetworkAccessManager(this)),
    temperatureCurve(new QwtPlotCurve("Temperature"))
{
    std::cout << "MainWindow constructor: Starting initialization..." << std::endl;

    if (!ui) {
        std::cerr << "Error: UI object is null!" << std::endl;
        exit(1); 
    }
    ui->setupUi(this);
    std::cout << "MainWindow constructor: UI setup completed." << std::endl;

    if (!networkManager) {
        std::cerr << "Error: Network manager is null!" << std::endl;
        exit(1); 
    }
    connect(ui->updateButton, &QPushButton::clicked, this, &MainWindow::fetchTemperatureStats);
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onStatsReply);

    std::cout << "MainWindow constructor: Signals connected." << std::endl;

    fetchCurrentTemperature();
}

MainWindow::~MainWindow() {
    delete ui; 
}

void MainWindow::fetchCurrentTemperature() {
    std::cout << "Fetching current temperature..." << std::endl;

    if (!networkManager) {
        std::cerr << "Network manager is not initialized." << std::endl;
        return;
    }

    QString url = QString("http://localhost:8080/current"); 

    QNetworkRequest request((QUrl(url))); 

    auto *reply = networkManager->get(request);

    if (!reply) {
        std::cerr << "Failed to create QNetworkReply object." << std::endl;
        return;
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cerr << "Network error: " << reply->errorString().toStdString() << std::endl;
            ui->currentTemperatureLabel->setText("Network error: " + reply->errorString());
        } else {
            std::cout << "Received current temperature reply." << std::endl;
            onCurrentReply(reply);
        }
        reply->deleteLater();
        return;
    });
}


void MainWindow::fetchTemperatureStats() {
    QDateTime startTime = ui->startDateTimeEdit->dateTime();
    QDateTime endTime = ui->endDateTimeEdit->dateTime();

    QString url = QString("http://localhost:8080/stats?start=%1&end=%2")
                    .arg(startTime.toSecsSinceEpoch())
                    .arg(endTime.toSecsSinceEpoch());

    QNetworkRequest request(url); 
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::cerr << "Stats fetch error: " << reply->errorString().toStdString() << std::endl;
            ui->statusLabel->setText("Error fetching stats.");
        } else {
            std::cout << "Stats fetch reply received." << std::endl;
            onStatsReply(reply);
        }
        reply->deleteLater();
    });
}


void MainWindow::onCurrentReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        std::cerr << "Error in current temperature reply: " << reply->errorString().toStdString() << std::endl;
        ui->currentTemperatureLabel->setText("Error fetching current temperature");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull() || !doc.isObject()) {
        std::cerr << "Invalid JSON format in current temperature reply." << std::endl;
        ui->currentTemperatureLabel->setText("Invalid data received.");
        return;
    }

    QJsonObject obj = doc.object();
    double temperature = obj["temperature"].toDouble();
    ui->currentTemperatureLabel->setText(QString::number(temperature, 'f', 2) + " °C");
}

void MainWindow::onStatsReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        std::cerr << "Error in stats reply: " << reply->errorString().toStdString() << std::endl;
        ui->statusLabel->setText("Error fetching stats data");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull() || !doc.isArray()) {
        std::cerr << "Invalid JSON format in stats reply." << std::endl;
        ui->statusLabel->setText("Invalid data received.");
        return;
    }

    QJsonArray array = doc.array();
    QVector<double> timestamps, temperatures;
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        timestamps.append(obj["timestamp"].toDouble());
        temperatures.append(obj["temperature"].toDouble());
    }

    updateGraph(timestamps, temperatures);
    ui->statusLabel->setText("Stats updated successfully.");
}

void MainWindow::updateGraph(const QVector<double> &timestamps, const QVector<double> &temperatures) {
    QVector<double> xData, yData;

    for (int i = 0; i < timestamps.size(); ++i) {
        xData.append(static_cast<double>(timestamps[i]));
        yData.append(temperatures[i]);
    }

    temperatureCurve->setSamples(xData, yData);
    temperatureCurve->attach(ui->qwtPlot);
    ui->qwtPlot->replot();
    std::cout << "Graph updated successfully." << std::endl;
}
