mkdir build
cd build

cmake .. -G "MinGW Makefiles"

cmake --build .

start emulator.exe
start server.exe

cd ..
start python src\client.py  
