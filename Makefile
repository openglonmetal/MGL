#-include config.mk

# first, with brew already installed (see https://brew.sh), do 'make install-pkgdeps'
# you can configure below, but it should be ok as is
# then 'make -j test' or 'make -j dbg' if the test crashes

# Find SDK path via xcode-select, backwards compatible with Xcode vers < 4.5
# on M1 monterey, comment out the following line
SDK_ROOT = $(shell xcode-select -p)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk

# with installed spirv_headers and spirv_cross
# spirv_headers_include_path ?= /usr/local/include
# spirv_cross_include_path ?= /usr/local/include/spirv
# spirv_cross_1_2_include_path ?= /usr/local/include/spirv/1.2
# spirv_cross_config_include_path ?= external/SPIRV-Cross
# spirv_cross_lib_path ?= /usr/local/lib

# with uninstalled spirv_headers and spirv_cross
# uncomment the following lines 
spirv_cross_1_2_include_path ?= SPIRV-Headers/include/spirv/1.2
spirv_cross_config_include_path ?= SPIRV-Cross
spirv_cross_lib_path ?= SPIRV-Cross/build

glslang_path ?= glslang
glslang_include_path ?= $(glslang_path)/build/include/glslang $(glslang_path)/glslang/Include
glslang_lib_path ?= $(glslang_path)/build/glslang $(glslang_path)/build/OGLCompilersDLL $(glslang_path)/build/glslang/OSDependent/Unix $(glslang_path)/build/StandAlone $(glslang_path)/build/SPIRV

# build dir
build_dir ?= build

# --
# no need to tweak after this line, hopefully

default: lib

brew_prefix := $(shell brew --prefix)

# mgl

mgl_srcs_c := $(wildcard MGL/src/*.c)
mgl_srcs_objc := $(wildcard MGL/src/*.m)

mgl_objs := $(mgl_srcs_c:.c=.o) $(mgl_srcs_cpp:.cpp=.o)
mgl_objs := $(addprefix $(build_dir)/,$(mgl_objs))

mgl_arc_objs := $(mgl_srcs_objc:.m=.o)
mgl_arc_objs := $(addprefix $(build_dir)/arc/,$(mgl_arc_objs))

deps += $(mgl_objs:.o=.d)
deps += $(mgl_arc_objs:.o=.d)

mgl_lib := $(build_dir)/libmgl.dylib

$(mgl_lib): $(mgl_objs) $(mgl_arc_objs)
	@mkdir -p $(dir $@)
	$(CXX) -dynamiclib -o $@ $^ $(LIBS)
	# loading dynamic library requires this
	ln -fs $(mgl_lib) .

# test

test_srcs_cpp := $(wildcard test_mgl/*.cpp)
test_objs := $(test_srcs_cpp:.cpp=.o)
test_deps := $(test_objs:.o=.d)
test_objs := $(addprefix $(build_dir)/,$(test_objs))
deps += $(test_objs:.o=.d)
test_exe := $(build_dir)/test
$(test_exe): $(mgl_lib)
$(test_exe): test_libs += $(shell pkg-config --libs glfw3)
$(test_objs): CFLAGS += -DTEST_MGL_GLFW $(shell pkg-config --cflags glfw3)
#$(test_exe): test_libs += $(shell pkg-config --libs sdl2)
#$(test_objs): CFLAGS += -DTEST_MGL_SDL $(shell pkg-config --cflags sdl2)

$(test_exe): $(test_objs)
	@mkdir -p $(dir $@)
	$(CXX) -o $@ $^ -L$(build_dir) -lmgl $(test_libs) -fsanitize=address

CFLAGS += -Wall #-Wunused-parameter #-Wextra
CFLAGS += -gfull -O0 -fsanitize=address
#CFLAGS += -03
LIBS += -fsanitize=address
CFLAGS += -arch $(shell uname -m)
CFLAGS += -I$(spirv_cross_1_2_include_path)
CFLAGS += -I$(spirv_cross_config_include_path)
#CFLAGS += -I$(brew_prefix)/opt/glslang/include/glslang/Include
CFLAGS += $(addprefix -I,$(glslang_include_path))
CFLAGS += $(shell pkg-config --cflags SPIRV-Tools)
CFLAGS += $(shell pkg-config --cflags glm)
CFLAGS += -IMGL/include
CFLAGS += -IMGL/include/GL # "glcorearb.h"
CFLAGS += -IMGL/SPIRV/SPIRV-Cross
CFLAGS += -DENABLE_OPT=0 -DSPIRV_CROSS_C_API_MSL=1 -DSPIRV_CROSS_C_API_GLSL=1 -DSPIRV_CROSS_C_API_CPP=1 -DSPIRV_CROSS_C_API_REFLECT=1
ifneq ($(SDK_ROOT),)
CFLAGS += -isysroot $(SDK_ROOT)
endif

LIBS += -L$(spirv_cross_lib_path) -lspirv-cross-core -lspirv-cross-c -lspirv-cross-cpp -lspirv-cross-msl -lspirv-cross-glsl -lspirv-cross-hlsl -lspirv-cross-reflect
#LIBS += -L$(brew_prefix)/opt/glslang/lib
LIBS += $(addprefix -L,$(glslang_lib_path))
LIBS += -lglslang -lMachineIndependent -lGenericCodeGen -lOGLCompiler -lOSDependent -lglslang-default-resource-limits -lSPIRV
#LIBS += -framework OpenGL -framework CoreGraphics

# specific rules

#lib: $(build_dir)/libglfw.dylib
lib: $(mgl_lib)

test: $(test_exe)
	$(test_exe)

dbg: $(test_exe)
	lldb -o run $(test_exe)


# # glfw
# # not needed anymore
# glfw_srcs_c := $(addprefix glfw/src/,cocoa_time.c posix_module.c posix_thread.c)
# glfw_srcs_c += $(addprefix glfw/src/,context.c init.c input.c monitor.c platform.c vulkan.c window.c egl_context.c osmesa_context.c null_init.c null_monitor.c null_window.c null_joystick.c)
# glfw_srcs_objc := $(addprefix glfw/src/,cocoa_init.m cocoa_joystick.m cocoa_monitor.m cocoa_window.m mgl_context.m)
# glfw_objs := $(glfw_srcs_c:.c=.o) $(glfw_srcs_objc:.m=.o)
# glfw_objs := $(addprefix $(build_dir)/,$(glfw_objs))
# deps += $(glfw_objs:.o=.d)
# $(glfw_objs): CFLAGS += -D_GLFW_COCOA
# #$(build_dir)/libglfw.dylib: LIBS += -L$(build_dir) -lMGL
# glfw_lib := $(build_dir)/libglfw.dylib
# $(glfw_lib): $(mgl_lib)

# $(glfw_lib): $(glfw_objs)
# 	@mkdir -p $(dir $@)
# 	$(CC) -dynamiclib -o $@ $^ -L$(build_dir) -lmgl


# generic rules

$(build_dir)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -std=gnu17 -MMD $(CFLAGS) -c $< -o $@

$(build_dir)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -std=c++14 -MMD $(CFLAGS) -c $< -o $@

$(build_dir)/arc/%.o: %.m
	@mkdir -p $(dir $@)
	clang -fobjc-arc -fmodules -MMD $(CFLAGS) -c $< -o $@

$(build_dir)/%.o: %.m
	@mkdir -p $(dir $@)
	clang -fmodules -MMD $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(build_dir)

install-pkgdeps:
	brew install glm glslang spirv-tools glfw
	git submodule init
	git submodule update --depth 1
	(cd SPIRV-Cross && mkdir -p build && cd build && cmake .. && make)
	(cd external/glslang && mkdir -p build && cd build && cmake .. && make)

update-pkdeps:
	git submodule -q foreach git pull -q origin master

test-make:
	@echo $(glfw_objs)

.PHONY: default test dbg lib clean insall-pkgdeps test-make 

-include $(deps)
