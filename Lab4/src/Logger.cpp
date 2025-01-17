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

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif
#include <filesystem>

#ifdef _WIN32
HANDLE initializeSerialPort(const std::string& port) {
    HANDLE hSerial = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open port " << port << std::endl;
        return INVALID_HANDLE_VALUE;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting state of serial port" << std::endl;
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting serial port state" << std::endl;
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error setting timeouts" << std::endl;
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    std::cout << "Port " << port << " initialized successfully." << std::endl;
    return hSerial;
}
#else
int initializeSerialPort(const std::string& port) {
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd == -1) {
        perror("Failed to open port");
        return -1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("Error getting terminal attributes");
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error setting terminal attributes");
        close(fd);
        return -1;
    }

    std::cout << "Port " << port << " initialized successfully." << std::endl;
    return fd;
}
#endif

#ifdef _WIN32
std::string readFromSerial(HANDLE hSerial) {
    char buffer[64] = {0};
    DWORD bytesRead = 0;
    if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
        buffer[bytesRead] = '\0';
        return std::string(buffer);
    } else {
        std::cerr << "Error reading from serial port" << std::endl;
        return "";
    }
}
#else
std::string readFromSerial(float fd) {
    char buffer[64] = {0};
    float bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        return std::string(buffer);
    } else {
        perror("Error reading from serial port");
        return "";
    }
}
#endif

void logTemperature(const std::string& logFile, float temperature) {
    std::ofstream log(logFile, std::ios::app);
    std::time_t now = std::time(nullptr);
    log << now << " " << std::setprecision(3) << temperature << std::endl;
}

std::vector<std::pair<std::time_t, int>> filterLogData(const std::string& logFile, int maxAgeInSeconds) {
    std::ifstream inFile(logFile);
    std::vector<std::pair<std::time_t, int>> filteredData;
    std::string line;
    std::time_t now = std::time(nullptr);

    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::time_t timestamp;
        float value;
        if (iss >> timestamp >> value) {
            if (now - timestamp <= maxAgeInSeconds) {
                filteredData.emplace_back(timestamp, value);
            }
        }
    }
    return filteredData;
}

void calculateAndLogAverage(const std::string& inputLog, const std::string& outputLog, int intervalInSeconds) {
    auto filteredData = filterLogData(inputLog, intervalInSeconds);
    std::vector<float> temperatures;

    for (const auto& entry : filteredData) {
        temperatures.push_back(entry.second);
    }

    if (!temperatures.empty()) {
        float avg = std::accumulate(temperatures.begin(), temperatures.end(), 0.0) / temperatures.size();
        std::time_t now = std::time(nullptr);
        std::ofstream outLog(outputLog, std::ios::app);
        outLog << now << " " << avg << std::endl;
    }
}


void cleanOldData(const std::string& logFile, int maxAgeInSeconds) {
    auto filteredData = filterLogData(logFile, maxAgeInSeconds);
    std::ofstream outFile(logFile, std::ios::trunc);

    for (const auto& entry : filteredData) {
        outFile << entry.first << " " << entry.second << std::endl;
    }
}


int main() {
    const std::string port = 
    #ifdef _WIN32
            "COM8";
    #else
            "/dev/ttyS0";
    #endif
    const std::string logDir = "../logs";
    std::filesystem::create_directories(logDir);
    const std::string logFile = logDir + "/temperature.log";
    const std::string hourlyLogFile = logDir + "/hourly_average.log";
    const std::string dailyLogFile = logDir + "/daily_average.log";

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

    while (true) {
        auto data = readFromSerial(serialPort);
        if (!data.empty()) {
            try {
                float temperature = std::stof(data);
                logTemperature(logFile, temperature);

                cleanOldData(logFile, 24 * 3600);

                static auto lastHourlyCheck = std::chrono::system_clock::now();
                if (std::chrono::system_clock::now() - lastHourlyCheck >= std::chrono::hours(1)) {
                    calculateAndLogAverage(logFile, hourlyLogFile, 3600);
                    cleanOldData(hourlyLogFile, 30 * 24 * 3600); 
                    lastHourlyCheck = std::chrono::system_clock::now();
                }

                static auto lastDailyCheck = std::chrono::system_clock::now();
                if (std::chrono::system_clock::now() - lastDailyCheck >= std::chrono::hours(24)) {
                    calculateAndLogAverage(hourlyLogFile, dailyLogFile, 24 * 3600);
                    cleanOldData(dailyLogFile, 365 * 24 * 3600);
                    lastDailyCheck = std::chrono::system_clock::now();
                }

                std::cout << "Logged: " << temperature << std::endl;
            } catch (...) {
                std::cerr << "Failed to parse temperature." << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

#ifdef _WIN32
    CloseHandle(serialPort);
#else
    close(serialPort);
#endif
    return 0;
}
