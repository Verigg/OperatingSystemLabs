#include "background_process.h" 
#include <iostream> 
#include <thread>
#include <chrono>

int main() {
    std::string program;
    std::string args; 
    std::string captureOutput; 


    std::cout << "Enter program to run: ";
    std::getline(std::cin, program); 

    std::cout << "Enter arguments (optional): ";
    std::getline(std::cin, args); 

    std::cout << "Capture output to console (yes/no): "; 
    std::getline(std::cin, captureOutput);

    bool capture = (captureOutput == "yes"); 

    auto pid = BackgroundProcess::run(program, args, capture); 
    if (!pid) { 
        std::cerr << "Failed to start program." << std::endl; 
        return 1; 
    }
    
    auto exitCode = BackgroundProcess::wait(*pid); 
    if (!exitCode) {
        std::cerr << "Failed to retrieve exit code." << std::endl;
        return 1;
    }

    std::cout << "Program exited with code: " << *exitCode << std::endl;
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}