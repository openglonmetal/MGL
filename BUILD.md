# CMake

This is not a production solution and is only enought instructions to get the libraries and test program compiled. Right now thought the test program doesn't actually work and an exception is thrown/seg faults. But hopefully this is enough for other people to come on board and help to get it working. 

## 1. Building Dependencies

This is done through a script in the external folder. the patched glfw library is already in this folder and is not cloned from anywhere.

- run `clone_external.sh`
- optionally edit the build_external.sh to add `-j 4` or `-j 8` etc to the end of the make commands to add more cores to the build. be wary of RAM explosions here. 
- run `build_external.sh`

## 2. Building MGL shared library and test program

In the root of the repository, run...
```
mkdir build
cd build
cmake ..
make -j 4
```
This is hard coded to look for the dependencies in the place where they were built. If you do `make install` the dependencies, then I think they will get picked up as well. 


# Xcode

Open the xcodeproj and build any of: 
- `MGL`: produces `libMGL.dylib`
- `GLFW`: produces `libMGL.dylib` and a patched GLFW that uses MGL.
- `test_mgl_glfw`: runs the tests

If you get errors such as `No account for team "..."` open the affected target and in the `Signing and Capabilities` tab select the correct team.
If there is no `Signing and Capabilities` tab open `Build Settings`, search for "signing", and select the correct team in `Development Team`.
