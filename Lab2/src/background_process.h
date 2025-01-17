#ifndef BACKGROUND_PROCESS_H
#define BACKGROUND_PROCESS_H

#include <string>
#include <optional>

namespace BackgroundProcess {

// Запускает программу в фоновом режиме.
// Возвращает идентификатор процесса или std::nullopt в случае ошибки.
// Если `captureOutput` равно true, вывод дочернего процесса отображается в консоли родительского процесса.
std::optional<int> run(const std::string& program, const std::string& args = "", bool captureOutput = false);

// Ожидает завершения программы и получает код завершения.
// Возвращает std::nullopt, если процесс не существует или произошла ошибка.
std::optional<int> wait(int pid);

}

#endif 
