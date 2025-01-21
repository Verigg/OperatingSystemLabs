mkdir build
cd build

cmake ../

cmake --build .

./emulator &
./logger &