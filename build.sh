rm -rf build

cd shaders
sh compileShaders.sh
cd ..

cmake -G "Ninja" -B build
cmake --build build
cd build
./Engine
