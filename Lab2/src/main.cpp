#include "background_process.h" // Подключаем заголовочный файл для работы с фоновыми процессами
#include <iostream> 
#include <thread>
#include <chrono>

int main() {
    std::string program;
    std::string args; 
    std::string captureOutput; 

    // Запрашиваем у пользователя название программы, аргументы программы (необязательно), нужно ли захватывать вывод в консоль
    std::cout << "Enter program to run: ";
    std::getline(std::cin, program); 

    std::cout << "Enter arguments (optional): ";
    std::getline(std::cin, args); 

    std::cout << "Capture output to console (yes/no): "; 
    std::getline(std::cin, captureOutput);

    bool capture = (captureOutput == "yes"); // Определяем, нужно ли захватывать вывод на основе ответа пользователя

    auto pid = BackgroundProcess::run(program, args, capture); // Запускаем программу в фоновом режиме и получаем PID
    if (!pid) { // Проверяем, удалось ли запустить программу
        std::cerr << "Failed to start program." << std::endl; 
        return 1; 
    }

    auto exitCode = BackgroundProcess::wait(*pid); // Ожидаем завершения программы и получаем код выхода
    if (!exitCode) {
        std::cerr << "Failed to retrieve exit code." << std::endl;
        return 1;
    }

    std::cout << "Program exited with code: " << *exitCode << std::endl; // Выводим код выхода программы
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0; // Успешное завершение программы
}
