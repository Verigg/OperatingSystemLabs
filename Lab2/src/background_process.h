#ifndef BACKGROUND_PROCESS_H
#define BACKGROUND_PROCESS_H

#include <string>
#include <optional>

namespace BackgroundProcess {

std::optional<int> run(const std::string& program, const std::string& args = "", bool captureOutput = false);

std::optional<int> wait(int pid);

}

#endif 
