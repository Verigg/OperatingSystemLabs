@echo off
setlocal

set REPO_DIR=%~dp0

cd /d %REPO_DIR%

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