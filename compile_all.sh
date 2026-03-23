# compile shaders
./compile_shaders.sh 

# compile C++ code
make clean && make -j$(nproc)

