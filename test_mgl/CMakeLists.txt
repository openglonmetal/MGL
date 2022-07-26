project(test_mgl)
cmake_minimum_required(VERSION 3.18)
#set(CMAKE_OSX_ARCHITECTURES "arm64")

set(CMAKE_EXE_LINKER_FLAGS "-framework Quartzcore -framework IOKit -framework Cocoa -framework Foundation")

add_executable(test_mgl main.cpp)

target_compile_definitions(test_mgl PUBLIC 
                                TEST_MGL_GLFW
                                ENABLE_OPT=0 
                                SPIRV_CROSS_C_API_MSL=1
                                SPIRV_CROSS_C_API_GLSL=1
                                SPIRV_CROSS_C_API_CPP=1
                                SPIRV_CROSS_C_API_REFLECT=1)


target_include_directories(test_mgl PUBLIC 
                            ../MGL/include
                            ../external/OpenGL-Registry/api
                            ../external/glfw/include
                            ${GLM_INCLUDE_DIRS}/../include)

target_link_libraries(test_mgl
                        mgl
                        ${SPIRVLIB}
                        ${SPIRVCROSS}
                        ${SPIRVCROSSC}
                        ${SPIRVCROSSCPP}
                        ${SPIRVCROSSMSL}
                        ${SPIRVCROSSGLSL}
                        ${SPIRVCROSSHLSL}
                        ${SPIRVCROSSREFLECT}
                        ${GLSLANG}
                        ${MACHINEINDEPENDENT}
                        ${GENERICCODEGEN}
                        ${OGLCOMPILER}
                        ${OSDEPENDENT}
                        ${GLSLANGDEFAULT}
                        ${SPIRVTOOLS}
                        ${SPIRVTOOLSOPT}
                        ${SPVREMAPPER}
                        ${CUSTOMGLFW}      
                        )


