#include "background_process.h"
#include <iostream>
#include <vector>

// Проверяем, на какой системе выполняется код: Windows или POSIX (Linux, macOS)
#ifdef _WIN32 
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#endif

// Объявление пространства имен для управления фоновыми процессами
namespace BackgroundProcess {

// Определение функции для Windows
#ifdef _WIN32
std::optional<int> run(const std::string& program, const std::string& args, bool captureOutput) {
    // Инициализация структур для запуска процесса
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = {};
    si.dwFlags |= STARTF_USESTDHANDLES; // Указание на использование стандартных дескрипторов

    HANDLE hStdOutRead = nullptr, hStdOutWrite = nullptr; // Дескрипторы для чтения и записи в pipe
    if (captureOutput) { // Проверяем, требуется ли захват вывода
        // Настройка атрибутов безопасности для pipe
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE; // Дескриптор может быть унаследован
        sa.lpSecurityDescriptor = nullptr;

        // Создание pipe для захвата вывода
        if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0) ||
            !SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
            return std::nullopt; // Возвращаем null, если не удалось создать pipe
        }
        // Перенаправляем стандартный вывод и ошибку на pipe
        si.hStdOutput = hStdOutWrite;
        si.hStdError = hStdOutWrite;
    }

    // Формируем команду для запуска
    std::string command = program + " " + args;

    // Запускаем процесс
    if (!CreateProcessA(nullptr, command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        // Закрываем дескрипторы pipe при неудаче
        if (captureOutput) {
            CloseHandle(hStdOutRead);
            CloseHandle(hStdOutWrite);
        }
        return std::nullopt; // Если запуск не удалось, возвращаем null
    }
    
    std::cout << "Program started with PID: " << pi.dwProcessId << std::endl; // Выводим PID запущенной программы
    
    if (captureOutput) {
        CloseHandle(hStdOutWrite); // Закрываем дескриптор записи после запуска

        char buffer[1024]; // Буфер для чтения данных из pipe
        DWORD bytesRead;
        // Читаем данные из pipe и выводим их на консоль
        while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Завершаем строку нулевым символом
            std::cout << buffer; // Выводим данные
        }
        CloseHandle(hStdOutRead); // Закрываем дескриптор чтения
    }

    CloseHandle(pi.hThread); // Закрываем дескрипторы потока и процесса
    CloseHandle(pi.hProcess);

    return static_cast<int>(pi.dwProcessId); // Возвращаем ID запущенного процесса
}

// Функция ожидания завершения процесса по его ID
std::optional<int> wait(int pid) {
    HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(pid)); 
    if (!hProcess) {
        return std::nullopt; // Если нельзя открыть процесс, возвращаем null
    }

    WaitForSingleObject(hProcess, INFINITE); // Ожидаем завершения процесса

    DWORD exitCode;
    // Получаем код завершения процесса
    if (GetExitCodeProcess(hProcess, &exitCode)) {
        CloseHandle(hProcess); // Закрываем дескриптор процесса
        return static_cast<int>(exitCode); // Возвращаем код завершения
    }

    CloseHandle(hProcess); // Закрываем дескриптор процесса
    return std::nullopt; // Если не удалось получить код завершения, возвращаем null
}
#else
// Определение функции для POSIX-систем (Linux / macOS)
std::optional<int> run(const std::string& program, const std::string& args, bool captureOutput) {
    int pipefd[2]; // Массив для дескрипторов pipe
    // Если требуется захват вывода, создаем pipe
    if (captureOutput && pipe(pipefd) == -1) {
        return std::nullopt; // Возвращаем null при ошибке создания pipe
    }

    pid_t pid = fork(); // Создаем новый процесс
    if (pid == -1) {
        if (captureOutput) {
            close(pipefd[0]); // Закрываем дескрипторы pipe в случае ошибки
            close(pipefd[1]);
        }
        return std::nullopt; // Возвращаем null при ошибке fork
    }
    if (pid == 0) {
        // Код выполняется в дочернем процессе
        if (captureOutput) {
            close(pipefd[0]); // Закрываем дескриптор чтения
            dup2(pipefd[1], STDOUT_FILENO); // Перенаправляем stdout на pipe
            dup2(pipefd[1], STDERR_FILENO); // Перенаправляем stderr на pipe
            close(pipefd[1]); // Закрываем дескриптор записи
        }

        std::string command = program + " " + args; // Формируем команду
        // Выполняем команду через shell
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(127); // Если execl не удалась, завершаем процесс с кодом 127
    }

    if (captureOutput) {
        close(pipefd[1]); // Закрываем дескриптор записи в родительском процессе
        char buffer[1024]; // Буфер для чтения данных из pipe
        ssize_t count;
        // Читаем данные из pipe и выводим их на консоль
        while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0'; // Завершаем строку нулевым символом
            std::cout << buffer; // Выводим данные
        }
        close(pipefd[0]); // Закрываем дескриптор чтения
    }

    return static_cast<int>(pid); // Возвращаем ID дочернего процесса
}

// Функция ожидания завершения процесса по его ID
std::optional<int> wait(int pid) {
    int status;
    pid_t result = waitpid(static_cast<pid_t>(pid), &status, 0); // Ожидаем завершения процесса
    if (result == -1) {
        return std::nullopt; // Если ошибка, возвращаем null
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status); // Если процесс завершился нормально, возвращаем код
    }

    return std::nullopt; // Если нет, возвращаем null
}
#endif

} // Конец пространства имен BackgroundProcess
