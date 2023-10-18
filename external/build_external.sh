cp ../MGL/include/MGLContext.h glfw/src
cp ../MGL/include/MGLRenderer.h glfw/src
cd SPIRV-Cross
mkdir build
cd build
cmake ..
make -j 4
cd ../..
cd SPIRV-Headers
mkdir build
cd build
cmake ..
make -j 4
cd ../..
cd SPIRV-Tools
mkdir build
cd build
cmake ..
make -j 4
cd ../..
cd glslang
mkdir build
cd build
cmake ..
make -j 4
cd ../..
cd glfw
mkdir build
cd build
cmake ..
make -j 4 glfw
cd ../..
