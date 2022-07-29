if [ ! -d "SPIRV-Tools" ]
then
    git clone https://github.com/KhronosGroup/SPIRV-Tools.git --depth 1
    cd SPIRV-Tools
else
    cd SPIRV-Tools
    git pull
fi

if [ ! -d "external/SPIRV-Headers" ]
then
    cd external
    git clone https://github.com/KhronosGroup/SPIRV-Headers.git --depth 1
    cd ..
else
    cd external/SPIRV-Headers
    git pull
    cd ../..
fi

if [ ! -d "build" ]
then
    mkdir build
    cd build
    cmake ..
else
    cd build
fi

make -j 4
