@echo off
setlocal

set REPO_DIR=%~dp0

cd /d %REPO_DIR%

echo Updating repository...
git pull @echo off
setlocal

set REPO_DIR=%~dp0

cd /d %REPO_DIR%

echo Updating repository...
git pull

for /d %%F in (Lab*) do (
    echo Building project in folder %%F...
    cd %%F
    mkdir build 2>nul
    cd build
    cmake ..
    cmake --build .
    cd ..
)

echo All projects have been built successfully!

endlocal
pause


for /d %%F in (Lab*) do (
    echo Building project in folder %%F...
    cd %%F
    mkdir build 2>nul
    cd build
    cmake ..
    cmake --build .
    cd ..
)

echo All projects have been built successfully!

endlocal
pause
