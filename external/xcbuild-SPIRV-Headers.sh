if [ ! -d "SPIRV-Headers" ]
then
    git clone https://github.com/KhronosGroup/SPIRV-Headers.git --depth 1
    cd SPIRV-Headers
else
    cd SPIRV-Headers
    git pull
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

