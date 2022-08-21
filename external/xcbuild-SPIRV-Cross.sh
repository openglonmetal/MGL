if [ ! -d "SPIRV-Cross" ]
then
    git clone https://github.com/r58Playz/SPIRV-Cross.git -b uniform-constants
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
