@echo off
setlocal

:: Путь к корневой директории проекта
set REPO_DIR=%~dp0

:: Перейти в папку с репозиторием
cd /d %REPO_DIR%

:: Обновить репозиторий с помощью Git
echo Updating repository...
git pull

:: Для каждой папки с лабораторной работой, например, Lab1, Lab2, Lab3...
for /d %%F in (Lab*) do (
    echo Building project in folder %%F...
    cd %%F
    mkdir build 2>nul
    cd build
    cmake -G "MinGW Makefiles" ..
    cmake --build .
    cd ..
    cd ..
)

echo All projects have been built successfully!

endlocal
pause
