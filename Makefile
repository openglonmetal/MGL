#-include config.mk

SHELL := /bin/bash

# Find SDK path via xcode-select, backwards compatible with Xcode vers < 4.5
# on M1 monterey, comment out the following line
SDK_ROOT = $(shell xcrun --sdk macosx --show-sdk-path)

# lets only install from external, devs complained about brew and we want the latest build from spirv
spirv_cross_include_path ?= ./external/SPIRV-Cross
spirv_cross_config_include_path ?= ./external/SPIRV-Cross
spirv_cross_lib_path ?= ./external/SPIRV-Cross/build

spirv_tools_include_path ?= ./external/SPIRV-Tools/include
spirv_tools_path ?= ./external/SPIRV-Tools/build

glslang_include_path ?= ./external/glslang/glslang/Include


#glslang_path ?= glslang
#glslang_include_path ?= $(glslang_path)/build/include/glslang $(glslang_path)/glslang/Include
#glslang_lib_path ?= $(glslang_path)/build/glslang $(glslang_path)/build/OGLCompilersDLL $(glslang_path)/build/glslang/OSDependent/Unix $(glslang_path)/build/StandAlone $(glslang_path)/build/SPIRV

# build dirs
build_dir ?= build
build_core_dir := $(build_dir)/core
build_es_dir := $(build_dir)/es

CFLAGS += -Wall #-Wunused-parameter #-Wextra
CFLAGS += -gfull
CFLAGS += -O2
#CFLAGS += -00
# Disable AddressSanitizer for production - causes crashes when loaded via dlopen()
#CFLAGS += -fsanitize=address
#LIBS += -fsanitize=address
CFLAGS += -arch $(shell uname -m)
LIBS += -arch $(shell uname -m)

LIBS += -F$(SDK_ROOT)/System/Library/Frameworks
LIBS += -framework Metal -framework OpenGL -framework Foundation

CFLAGS += -I$(spirv_cross_include_path)
CFLAGS += -I$(spirv_cross_config_include_path)
CFLAGS += -I$(spirv_tools_include_path)
CFLAGS += -I$(glslang_include_path)

# lets only install from external, devs complained about brew
# CFLAGS += $(shell pkg-config --cflags SPIRV-Tools)
# CFLAGS += $(shell pkg-config --cflags glm)

CFLAGS += -IMGL/include
CFLAGS += -IMGL/include/GL # "glcorearb.h"
CFLAGS += -IMGL/SPIRV/SPIRV-Cross
CFLAGS += -DENABLE_OPT=0 -DSPIRV_CROSS_C_API_MSL=1 -DSPIRV_CROSS_C_API_GLSL=1 -DSPIRV_CROSS_C_API_CPP=1 -DSPIRV_CROSS_C_API_REFLECT=1

# GLFW configuration for shared library build
CFLAGS += -I./external/glfw/include -I./external/glfw/src
CXXFLAGS += -I./external/glfw/include -I./external/glfw/src

# macOS specific compile definitions for GLFW
CFLAGS += -D_COCOA -D_GLFW_COCOA
CXXFLAGS += -D_COCOA -D_GLFW_COCOA

# GL_CORE SPECIFIC FLAGS
CFLAGS_GL_CORE := $(CFLAGS) -DMGL_GL_CORE
CXXFLAGS_GL_CORE := $(CXXFLAGS) -DMGL_GL_CORE

# GL_ES SPECIFIC FLAGS
CFLAGS_GL_ES := $(CFLAGS) -DMGL_GL_ES
CXXFLAGS_GL_ES := $(CXXFLAGS) -DMGL_GL_ES

# Add CoreFoundation framework headers for GLFW Objective-C compilation
GLFW_FRAMEWORKS = -framework Cocoa -framework CoreFoundation -framework CoreGraphics \
                  -framework IOKit -framework Foundation -framework QuartzCore \
                  -framework Metal -framework OpenGL

# GLFW sources for shared library build - macOS specific configuration
GLFW_SRC_DIR = external/glfw/src
GLFW_C_SOURCES = $(GLFW_SRC_DIR)/context.c \
                $(GLFW_SRC_DIR)/init.c \
                $(GLFW_SRC_DIR)/input.c \
                $(GLFW_SRC_DIR)/monitor.c \
                $(GLFW_SRC_DIR)/vulkan.c \
                $(GLFW_SRC_DIR)/window.c \
                $(GLFW_SRC_DIR)/osmesa_context.c \
                $(GLFW_SRC_DIR)/egl_context.c \
                $(GLFW_SRC_DIR)/posix_thread.c \
                $(GLFW_SRC_DIR)/posix_module.c \
                $(GLFW_SRC_DIR)/cocoa_time.c \
                $(GLFW_SRC_DIR)/platform.c

GLFW_M_SOURCES = $(GLFW_SRC_DIR)/cocoa_init.m \
                $(GLFW_SRC_DIR)/cocoa_joystick.m \
                $(GLFW_SRC_DIR)/cocoa_monitor.m \
                $(GLFW_SRC_DIR)/cocoa_window.m \
                $(GLFW_SRC_DIR)/mgl_context.m

# Simplified GLFW object paths - use a flat structure for easier building
GLFW_BUILD_DIR = $(build_dir)/glfw
GLFW_C_OBJS = $(GLFW_C_SOURCES:$(GLFW_SRC_DIR)/%.c=$(GLFW_BUILD_DIR)/%.o)

GLFW_M_OBJS = $(GLFW_M_SOURCES:$(GLFW_SRC_DIR)/%.m=$(GLFW_BUILD_DIR)/%.o)
glfw_objs = $(GLFW_C_OBJS) $(GLFW_M_OBJS)

ifneq ($(SDK_ROOT),)
CFLAGS_GL_CORE += -isysroot $(SDK_ROOT)
CFLAGS_GL_ES += -isysroot $(SDK_ROOT)
endif

LIBS += -L$(spirv_cross_lib_path) -lspirv-cross-core -lspirv-cross-c -lspirv-cross-cpp -lspirv-cross-msl -lspirv-cross-glsl -lspirv-cross-hlsl -lspirv-cross-reflect
# Use static libraries from external/glslang instead of homebrew
LIBS += external/glslang/build/glslang/libglslang.a external/glslang/build/glslang/libMachineIndependent.a external/glslang/build/glslang/libGenericCodeGen.a external/glslang/build/glslang/OSDependent/Unix/libOSDependent.a external/glslang/build/glslang/libglslang-default-resource-limits.a external/glslang/build/SPIRV/libSPIRV.a
LIBS += -L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib
LIBS += -lc++

# add all the SPIRV libs
SPIRV_LIBS := $(wildcard external/SPIRV-Cross/build/libspirv*.a)
LIBS += $(SPIRV_LIBS)

GLSL_LIBS := $(wildcard external/glslang/build/glslang/lib*.a)
LIBS += $(GLSL_LIBS)

# SPIRV-Tools
LIBS += external/SPIRV-Tools/build/source/lint/libSPIRV-Tools-lint.a
LIBS += external/SPIRV-Tools/build/source/reduce/libSPIRV-Tools-reduce.a
LIBS += external/SPIRV-Tools/build/source/diff/libSPIRV-Tools-diff.a
LIBS += external/SPIRV-Tools/build/source/libSPIRV-Tools.a
LIBS += external/SPIRV-Tools/build/source/link/libSPIRV-Tools-link.a
LIBS += external/SPIRV-Tools/build/source/opt/libSPIRV-Tools-opt.a


# --
# no need to tweak after this line, hopefully

default: lib

brew_prefix := $(shell brew --prefix)

# mgl
#mgl_srcs_c := $(wildcard MGL/src/*.c)
mgl_srcs_c := $(filter-out %/gl_core.c  %/gl_es.c, $(wildcard MGL/src/*.c))

mgl_srcs_objc := $(wildcard MGL/src/*.m)

mgl_core_c := MGL/src/gl_core.c
mgl_es_c := MGL/src/gl_es.c

mgl_core_obj := $(mgl_core_c:.c=.o)
mgl_core_obj := $(addprefix $(build_core_dir)/,$(mgl_core_obj))

mgl_es_obj := $(mgl_es_c:.c=.o)
mgl_es_obj := $(addprefix $(build_es_dir)/,$(mgl_es_obj))

# core objs
mgl_core_objs := $(mgl_srcs_c:.c=.o) $(mgl_srcs_cpp:.cpp=.o)
mgl_core_objs := $(addprefix $(build_core_dir)/,$(mgl_core_objs))

mgl_core_objs := $(mgl_srcs_c:.c=.o) $(mgl_srcs_cpp:.cpp=.o)
mgl_core_objs := $(addprefix $(build_core_dir)/,$(mgl_core_objs))

mgl_core_arc_objs := $(mgl_srcs_objc:.m=.o)
mgl_core_arc_objs := $(addprefix $(build_core_dir)/arc/,$(mgl_core_arc_objs))

# es objs
mgl_es_objs := $(mgl_srcs_c:.c=.o) $(mgl_srcs_cpp:.cpp=.o)
mgl_es_objs := $(addprefix $(build_es_dir)/,$(mgl_es_objs))

mgl_es_objs := $(mgl_srcs_c:.c=.o) $(mgl_srcs_cpp:.cpp=.o)
mgl_es_objs := $(addprefix $(build_es_dir)/,$(mgl_es_objs))

mgl_es_arc_objs := $(mgl_srcs_objc:.m=.o)
mgl_es_arc_objs := $(addprefix $(build_es_dir)/arc/,$(mgl_es_arc_objs))


# Define the directories and repositories
EXT_DIRS = ./external/OpenGL-Registry \
           ./external/SPIRV-Cross \
           ./external/SPIRV-Headers \
           ./external/SPIRV-Tools \
           ./external/glslang \
           ./external/ezxml

REPOS = https://github.com/KhronosGroup/OpenGL-Registry.git \
        https://github.com/KhronosGroup/SPIRV-Cross.git \
        https://github.com/KhronosGroup/SPIRV-Headers.git \
        https://github.com/KhronosGroup/SPIRV-Tools.git \
        https://github.com/KhronosGroup/glslang.git \
        https://github.com/lxfontes/ezxml.git

# Simplified index_of function - find position of directory in EXT_DIRS
define index_of
$(strip $(1))
endef

# Function to get the corresponding repository URL for a directory
# Simplified mapping for common directories
define get_repo_url
$(if $(filter $(1),./external/OpenGL-Registry),https://github.com/KhronosGroup/OpenGL-Registry.git, \
$(if $(filter $(1),./external/SPIRV-Cross),https://github.com/KhronosGroup/SPIRV-Cross.git, \
$(if $(filter $(1),./external/SPIRV-Headers),https://github.com/KhronosGroup/SPIRV-Headers.git, \
$(if $(filter $(1),./external/SPIRV-Tools),https://github.com/KhronosGroup/SPIRV-Tools.git, \
$(if $(filter $(1),./external/glslang),https://github.com/KhronosGroup/glslang.git, \
https://github.com/lxfontes/ezxml.git)))))
endef

# Function to check if a directory exists, and if not, clone it
define check_and_clone
	@echo "Resolving directory $(1)..."; \
	INDEX=$(call index_of,$(1)); \
	REPO=$(call get_repo_url,$(1)); \
	echo "INDEX calculated: $$INDEX"; \
	echo "REPO resolved: $$REPO"; \
	if [ ! -d $(1) ]; then \
		echo "Cloning from $$REPO into $(1)..."; \
		git clone $$REPO $(1) --depth 1; \
	else \
		echo "$(1) already exists, skipping."; \
	fi
endef

# Use the `check_and_clone` function for each directory
$(EXT_DIRS):
	$(call check_and_clone,$@)


deps += $(mgl_objs:.o=.d)
deps += $(mgl_core_obj:.o=.d)
deps += $(mgl_es_obj:.o=.d)
deps += $(mgl_arc_objs:.o=.d)


mgl_lib := $(build_dir)/libmgl.dylib
mgl_es_lib := $(build_dir)/libmgl_es.dylib

mgl_toolchain_obj := $(build_dir)/MGL/src/mgl_toolchain.o
mgl_toolchain_lib := $(build_dir)/libmgl_toolchain.a

$(mgl_lib): $(mgl_core_objs) $(mgl_core_arc_objs) $(mgl_gl_obj)
	@mkdir -p $(dir $@)
	$(CC) -D$(CFLAGS_GL_CORE) -dynamiclib -o $@ $^ $(LIBS)
	# loading dynamic library requires this
	ln -fs $(mgl_lib) .

$(mgl_es_lib): $(mgl_es_objs) $(mgl_es_arc_objs) $(mgl_es_obj)
	@mkdir -p $(dir $@)
	$(CC) -D$(CFLAGS_GL_ES) -dynamiclib -o $@ $^ $(LIBS)
	# loading dynamic library requires this
	ln -fs $(mgl_es_lib) .


$(mgl_toolchain_lib): $(mgl_toolchain_obj)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

# Build GLFW shared library from pre-built static library
$(build_dir)/libglfw.dylib: external/glfw/build/src/libglfw3.a $(mgl_lib)
	@echo "Creating GLFW shared library from static library..."
	@mkdir -p $(dir $@)
	$(CC) -shared -fPIC -dynamiclib \
		-Wl,-force_load,$(word 1,$^) \
		-L$(build_dir) -lmgl \
		-o $@ \
		$(GLFW_FRAMEWORKS) \
		-install_name @rpath/libglfw.dylib
	@echo "✅ GLFW shared library built: $@"
	@echo "This enables compatibility with Minecraft mods and Prism Launcher"


# specific rules

lib: $(mgl_lib) $(mgl_es_lib) $(build_dir)/libglfw.dylib

toolchain: $(mgl_toolchain_lib)

test: $(test_exe)
	$(test_exe)

dbg: $(test_exe)
	lldb -o run $(test_exe)


# generic rules

#
# core build
#
$(build_core_dir)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -MMD $(CFLAGS_GL_CORE) -c $< -o $@

#-std=gnu17 
$(build_core_dir)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -MMD $(CXXFLAGS_GL_CORE) -c $< -o $@

#-std=c++14
$(build_core_dir)/arc/%.o: %.m
	@mkdir -p $(dir $@)
	clang -fobjc-arc -fmodules -MMD $(CFLAGS_GL_CORE) \
		-framework Cocoa -framework CoreFoundation -framework CoreGraphics \
		-framework IOKit -framework Foundation -framework QuartzCore \
		-framework Metal -framework OpenGL \
		-c $< -o $@

$(build_core_dir)/%.o: %.m
	@mkdir -p $(dir $@)
	clang -fmodules -MMD $(CFLAGS_GL_CORE) -c $< -o $@


#
# es build
#
$(build_es_dir)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -MMD $(CFLAGS_GL_ES) -c $< -o $@

#-std=gnu17
$(build_es_dir)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -MMD $(CXXFLAGS_GL_ES) -c $< -o $@

#-std=c++14
$(build_es_dir)/arc/%.o: %.m
	@mkdir -p $(dir $@)
	clang -fobjc-arc -fmodules -MMD $(CFLAGS_GL_ES) \
		-framework Cocoa -framework CoreFoundation -framework CoreGraphics \
		-framework IOKit -framework Foundation -framework QuartzCore \
		-framework Metal -framework OpenGL \
		-c $< -o $@

$(build_dir)/%.o: %.m
	@mkdir -p $(dir $@)
	clang -fmodules -MMD $(CXXFLAGS_GL_ES) -c $< -o $@




# GLFW-specific build rules with simplified flat directory structure
$(GLFW_BUILD_DIR)/%.o: $(GLFW_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -MMD $(CFLAGS) -c $< -o $@

$(GLFW_BUILD_DIR)/%.o: $(GLFW_SRC_DIR)/%.m
	@mkdir -p $(dir $@)
	clang -fno-objc-arc -fmodules -MMD $(CFLAGS) $(GLFW_FRAMEWORKS) -c $< -o $@

clean:
	rm -rf $(build_dir)
	rm -f libmgl.dylib
	rm -f libmgl_es.dylib
	rm -f libglfw.dylib

install-pkgdeps: download-pkgdeps compile-pkgdeps

download-pkgdeps:
	brew install glm glslang spirv-tools glfw
	git submodule init
	git submodule update --depth 1

compile-pkgdeps:
	(cd SPIRV-Cross && mkdir -p build && cd build && cmake .. && make)

update-pkdeps:
	git submodule -q foreach git pull -q origin master

test-make:
	@echo $(glfw_objs)

.PHONY: default test dbg lib clean insall-pkgdeps test-make 

-include $(deps)
