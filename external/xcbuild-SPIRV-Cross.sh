if [ ! -d "SPIRV-Cross" ]
then
    git clone https://github.com/KhronosGroup/SPIRV-Cross.git --depth 1
    cd SPIRV-Cross
else
    cd SPIRV-Cross
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
