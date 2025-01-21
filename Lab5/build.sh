mkdir build
cd build

cmake ../

cmake --build .

./emulator &
./logger &
python src\client.py  