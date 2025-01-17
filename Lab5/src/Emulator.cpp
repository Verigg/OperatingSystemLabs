#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <iomanip>
#include "SerialPort.h".
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
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
