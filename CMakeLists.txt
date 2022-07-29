project(mgl)
cmake_minimum_required(VERSION 3.18)
set(CMAKE_OSX_ARCHITECTURES "arm64")

FILE(GLOB MyCSources MGL/src/*.c)
FILE(GLOB MyMSources MGL/src/*.m)

add_library(mgl SHARED ${MyCSources} ${MyMSources})

target_include_directories(mgl PUBLIC MGL/include MGL/include/GL)

set_source_files_properties(MGL/src/MGLRenderer.m PROPERTIES COMPILE_FLAGS "-x objective-c")
set_source_files_properties(MGL/src/MGLTextures.m PROPERTIES COMPILE_FLAGS "-x objective-c")

set(CMAKE_C_FLAGS "-fobjc-arc")

target_compile_definitions(mgl PUBLIC 
                                ENABLE_OPT=0 
                                SPIRV_CROSS_C_API_MSL=1
                                SPIRV_CROSS_C_API_GLSL=1
                                SPIRV_CROSS_C_API_CPP=1
                                SPIRV_CROSS_C_API_REFLECT=1
                                )

set(CMAKE_SHARED_LINKER_FLAGS "-lc++ -framework Quartz -framework AppKit -framework Quartzcore -framework IOKit -framework Metal -framework ApplicationServices -framework CoreGraphics -framework Cocoa  -framework Foundation")

find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_library(GLSLANG glslang external/glslang/build/glslang)
find_library(GENERICCODEGEN GenericCodeGen external/glslang/build/glslang)
find_library(MACHINEINDEPENDENT MachineIndependent external/glslang/build/glslang)
find_library(OGLCOMPILER OGLCompiler external/glslang/build/OGLCompilersDLL)
find_library(OSDEPENDENT OSDependent external/glslang/build/glslang/OSDependent/Unix)

find_library(SPIRVCROSS spirv-cross-core external/SPIRV-Cross/build)
find_library(SPIRVCROSSC spirv-cross-c external/SPIRV-Cross/build)
find_library(SPIRVCROSSCPP spirv-cross-cpp external/SPIRV-Cross/build)
find_library(SPIRVCROSSMSL spirv-cross-msl external/SPIRV-Cross/build)
find_library(SPIRVCROSSGLSL spirv-cross-glsl external/SPIRV-Cross/build)
find_library(SPIRVCROSSHLSL spirv-cross-hlsl external/SPIRV-Cross/build)
find_library(SPIRVCROSSREFLECT spirv-cross-reflect external/SPIRV-Cross/build)
find_library(SPIRVCROSSUTIL spirv-cross-util external/SPIRV-Cross/build)

find_library(GLSLANGDEFAULT glslang-default-resource-limits external/glslang/build/StandAlone)

find_library(SPIRVLIB SPIRV external/glslang/build/SPIRV)
find_library(SPVREMAPPER SPVRemapper external/glslang/build/SPIRV)

find_library(SPIRVTOOLS SPIRV-Tools external/SPIRV-Tools/build/source)
find_library(SPIRVTOOLSOPT SPIRV-Tools-opt external/SPIRV-Tools/build/source/opt)
find_library(CUSTOMGLFW glfw3 external/glfw/build/src)

target_include_directories(${PROJECT_NAME} PUBLIC  
                            external/SPIRV-Cross/include
                            external/SPIRV-Cross
                            external/glslang/glslang/Include
                            external/SPIRV-Tools/include
                            glfwlib/include
                            )

MESSAGE(STATUS "Found glslang in: ${GLSLANG}")
MESSAGE(STATUS "Found spirv-cross-core in: ${SPIRVCROSS}")
MESSAGE(STATUS "Found spirv-cross-c in: ${SPIRVCROSSC}")
MESSAGE(STATUS "Found spirv-cross-cpp in: ${SPIRVCROSSCPP}")
MESSAGE(STATUS "Found spirv-cross-msl in: ${SPIRVCROSSMSL}")
MESSAGE(STATUS "Found spirv-cross-glsl in: ${SPIRVCROSSGLSL}")
MESSAGE(STATUS "Found spirv-cross-hlsl in: ${SPIRVCROSSHLSL}")
MESSAGE(STATUS "Found spirv-cross-reflect in: ${SPIRVCROSSREFLECT}")
MESSAGE(STATUS "Found machine independent in: ${MACHINEINDEPENDENT}")
MESSAGE(STATUS "Found generic in: ${GENERICCODEGEN}")
MESSAGE(STATUS "Found ogl compiiler in: ${OGLCOMPILER}")
MESSAGE(STATUS "Found os dep in: ${OSDEPENDENT}")
MESSAGE(STATUS "Found glslang def in: ${GLSLANGDEFAULT}")
MESSAGE(STATUS "Found spirv in: ${SPIRVLIB}")
MESSAGE(STATUS "Found spirvtools in: ${SPIRVTOOLS}")
MESSAGE(STATUS "Found spirvtoolsopt in: ${SPIRVTOOLSOPT}")
MESSAGE(STATUS "Found spv remapper in: ${SPVREMAPPER}")
MESSAGE(STATUS "Found custom glfw in: ${CUSTOMGLFW}")

target_link_libraries(${PROJECT_NAME} 
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
                        ${SPIRVLIB}
                        ${SPIRVTOOLSOPT}
                        ${SPVREMAPPER}
                        ${OPENGL_LIBRARIES}
                        )

add_subdirectory(test_mgl)
