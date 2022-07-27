#Building MGL with CMake

This is not a production solution and is only enought instructions to get the libraries and test program compiled. Right now thought the test program doesn't actually work and an exception is thrown/seg faults. But hopefully this is enough for other people to come on board and help to get it working. 

# 1. Building Dependencies

This is done through a script in the external folder. the patched glfw library is already in this folder and is not cloned from anywhere.

- run `clone_external.sh`
- optionally edit the build_external.sh to add `-j 4` or `-j 8` etc to the end of the make commands to add more cores to the build. be wary of RAM explosions here. 
- run `build_external.sh`

# 2. Building MGL shared library and test program

In the root of the repository, run...
```
mkdir build
cd build
cmake ..
make -j 4
```
This is hard coded to look for the dependencies in the place where they were built. If you do `make install` the dependencies, then I think they will get picked up as well. 

