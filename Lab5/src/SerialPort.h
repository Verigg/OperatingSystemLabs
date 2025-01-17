#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <string>

#ifdef _WIN32
#include <windows.h>
HANDLE initializeSerialPort(const std::string& port);
void writeToSerial(HANDLE hSerial, const std::string& data);
std::string readFromSerial(HANDLE hSerial);
#else
#include <termios.h>
#include <unistd.h>
int initializeSerialPort(const std::string& port);
void writeToSerial(int fd, const std::string& data);
std::string readFromSerial(int fd);
#endif

#endif 
