#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <sqlite3.h>
#include "httplib.h"
#include "SerialPort.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif
#include <filesystem>

// Функции работы с базой данных
void initializeDatabase(sqlite3* &db) {
    std::filesystem::create_directories("../database");
    if (sqlite3_open("../database/temperature.db", &db)) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << "\n";
        exit(1);
    }

    const char* createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS temperature (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER NOT NULL,
            value REAL NOT NULL
        );
    )";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to create table: " << errMsg << "\n";
        sqlite3_free(errMsg);
        exit(1);
    }
}

void logTemperatureToDatabase(sqlite3* db, float temperature) {
    const char* insertSQL = "INSERT INTO temperature (timestamp, value) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, static_cast<int>(std::time(nullptr)));
    sqlite3_bind_double(stmt, 2, temperature);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(stmt);
}

std::vector<std::pair<std::time_t, float>> queryTemperature(sqlite3* db, int startTime, int endTime) {
    const char* querySQL = "SELECT timestamp, value FROM temperature WHERE timestamp BETWEEN ? AND ?;";
    sqlite3_stmt* stmt;
    std::vector<std::pair<std::time_t, float>> results;

    if (sqlite3_prepare_v2(db, querySQL, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return results;
    }

    sqlite3_bind_int(stmt, 1, startTime);
    sqlite3_bind_int(stmt, 2, endTime);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::time_t timestamp = sqlite3_column_int(stmt, 0);
        float value = static_cast<float>(sqlite3_column_double(stmt, 1));
        results.emplace_back(timestamp, value);
    }

    sqlite3_finalize(stmt);
    return results;
}

// HTTP-сервер
void startServer(sqlite3* db) {
    httplib::Server svr;

    svr.Get("/current", [db](const httplib::Request& req, httplib::Response& res) {
        auto results = queryTemperature(db, std::time(nullptr) - 60, std::time(nullptr));
        if (results.empty()) {
            res.set_content("{\"error\": \"No Data\"}", "application/json");
        } else {
            float current_temperature = results.back().second; // Последняя температура
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << current_temperature;
            res.set_content("{\"temperature\": " + oss.str() + "}", "application/json");
        }
    });

    svr.Get("/stats", [db](const httplib::Request& req, httplib::Response& res) {
        try {
            int startTime = std::stoi(req.get_param_value("start"));
            int endTime = std::stoi(req.get_param_value("end"));

            auto results = queryTemperature(db, startTime, endTime);
            std::ostringstream oss;

            // Создаем JSON-массив
            oss << "[";
            for (size_t i = 0; i < results.size(); ++i) {
                oss << "{\"timestamp\": " << results[i].first
                    << ", \"temperature\": " << std::fixed << std::setprecision(2) << results[i].second << "}";
                if (i < results.size() - 1) {
                    oss << ",";
                }
            }
            oss << "]";

            res.set_content(oss.str(), "application/json");
        } catch (const std::exception& e) {
            res.set_content("{\"error\": \"Invalid request parameters\"}", "application/json");
        }
    });

    std::cout << "Server started at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
}

int main() {
    const std::string port = 
    #ifdef _WIN32
            "COM8";
    #else
            "/dev/ttyS0";
    #endif

    sqlite3* db;
    initializeDatabase(db);

    auto serialPort = initializeSerialPort(port);
    if (
        #ifdef _WIN32
                serialPort == INVALID_HANDLE_VALUE
        #else
                serialPort == -1
        #endif
    ) {
        return 1;
    }

    std::thread serverThread(startServer, db);

    while (true) {
        auto data = readFromSerial(serialPort);
        if (!data.empty()) {
            try {
                float temperature = std::stof(data);
                logTemperatureToDatabase(db, temperature);
                std::cout << "Logged: " << std::fixed << std::setprecision(2) << temperature << "\n";
            } catch (...) {
                std::cerr << "Failed to parse temperature.\n";
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

#ifdef _WIN32
    CloseHandle(serialPort);
#else
    close(serialPort);
#endif
    sqlite3_close(db);
    serverThread.join();

    return 0;
}
