mkdir build
cd build

cmake .. -G "MinGW Makefiles"

cmake --build .

start emulator.exe
start logger.exe

cd ..