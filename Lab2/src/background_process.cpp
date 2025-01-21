#include "background_process.h"
#include <iostream>
#include <vector>

#ifdef _WIN32 
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#endif

namespace BackgroundProcess {

#ifdef _WIN32
std::optional<int> run(const std::string& program, const std::string& args, bool captureOutput) {

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = {};
    si.dwFlags |= STARTF_USESTDHANDLES; 

    HANDLE hStdOutRead = nullptr, hStdOutWrite = nullptr; 
    if (captureOutput) { 

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE; 
        sa.lpSecurityDescriptor = nullptr;


        if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0) ||
            !SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
            return std::nullopt; 
        }

        si.hStdOutput = hStdOutWrite;
        si.hStdError = hStdOutWrite;
    }


    std::string command = program + " " + args;


    if (!CreateProcessA(nullptr, command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        if (captureOutput) {
            CloseHandle(hStdOutRead);
            CloseHandle(hStdOutWrite);
        }
        return std::nullopt; 
    }
    
    std::cout << "Program started with PID: " << pi.dwProcessId << std::endl; 
    
    if (captureOutput) {
        CloseHandle(hStdOutWrite);

        char buffer[1024]; 
        DWORD bytesRead;

        while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
            buffer[bytesRead] = '\0'; 
            std::cout << buffer; 
        }
        std::cout << std::endl; 
        CloseHandle(hStdOutRead); 
    }

    CloseHandle(pi.hThread); 
    CloseHandle(pi.hProcess);

    return static_cast<int>(pi.dwProcessId); 
}


std::optional<int> wait(int pid) {
    HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(pid)); 
    if (!hProcess) {
        return std::nullopt; 
    }

    WaitForSingleObject(hProcess, INFINITE);

    DWORD exitCode;

    if (GetExitCodeProcess(hProcess, &exitCode)) {
        CloseHandle(hProcess); 
        return static_cast<int>(exitCode); 
    }

    CloseHandle(hProcess); 
    return std::nullopt; 
}
#else

std::optional<int> run(const std::string& program, const std::string& args, bool captureOutput) {
    int pipefd[2]; 

    if (captureOutput && pipe(pipefd) == -1) {
        return std::nullopt; 
    }

    pid_t pid = fork(); 
    if (pid == -1) {
        if (captureOutput) {
            close(pipefd[0]);
            close(pipefd[1]);
        }
        return std::nullopt; 
    }
    if (pid == 0) {
        if (captureOutput) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO); 
            dup2(pipefd[1], STDERR_FILENO); 
            close(pipefd[1]); 
        }

        std::string command = program + " " + args; 

        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(127); 
    }

    std::cout << "Program started with PID: " << pid << std::endl; 

    if (captureOutput) {
        close(pipefd[1]);
        char buffer[1024];
        ssize_t count;

        while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            std::cout << buffer; 
        }
        std::cout << std::endl;
        close(pipefd[0]); 
    }

    return static_cast<int>(pid); 
}


std::optional<int> wait(int pid) {
    int status;
    pid_t result = waitpid(static_cast<pid_t>(pid), &status, 0); 
    if (result == -1) {
        return std::nullopt; 
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status); 
    }

    return std::nullopt; 
}
#endif

} 
