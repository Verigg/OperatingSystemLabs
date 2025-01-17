#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

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
        std::cerr << "Error setting timeouts\n";
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
void writeToSerial(HANDLE hSerial, const std::string& data) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data.c_str(), data.size(), &bytesWritten, nullptr)) {
        std::cerr << "Error writing to serial port" << std::endl;
    }
}
#else
void writeToSerial(int fd, const std::string& data) {
    ssize_t bytesWritten = write(fd, data.c_str(), data.size());
    if (bytesWritten == -1) {
        perror("Error writing to serial port");
    }
}
#endif

int main() {
    const std::string port = 
#ifdef _WIN32
        "COM7";
#else
        "/dev/ttyS1"; // Замените на ваш порт в POSIX
#endif

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

    std::srand(std::time(nullptr));

    float temperature = 20.0; 
    const float minDelta = -0.5; 
    const float maxDelta = 0.5;  

    while (true) {
        float delta = minDelta + static_cast<float>(std::rand()) / RAND_MAX * (maxDelta - minDelta);
        temperature += delta;

        if (temperature < 15.0) temperature = 15.0;
        if (temperature > 30.0) temperature = 30.0;

        std::string data = std::to_string(temperature) + "\n";
        writeToSerial(serialPort, data);

        std::time_t now = std::time(nullptr);
        std::cout << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
                  << " " << temperature << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

#ifdef _WIN32
    CloseHandle(serialPort);
#else
    close(serialPort);
#endif
    return 0;
}
