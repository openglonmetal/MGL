set SDKROOT=`xcrun --show-sdk-path`

cp ../MGL/include/MGLContext.h glfw/src
cp ../MGL/include/MGLRenderer.h glfw/src
cd SPIRV-Tools
mkdir build
cd build
cmake ..
make -j 4
cd ../..
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
cd glslang
./update_glslang_sources.py
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
