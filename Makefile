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

# build dir
build_dir ?= build

CFLAGS += -Wall #-Wunused-parameter #-Wextra
CFLAGS += -gfull
CFLAGS += -O2
#CFLAGS += -00
CFLAGS += -fsanitize=address
LIBS += -fsanitize=address
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

ifneq ($(SDK_ROOT),)
CFLAGS += -isysroot $(SDK_ROOT)
endif

LIBS += -L$(spirv_cross_lib_path) -lspirv-cross-core -lspirv-cross-c -lspirv-cross-cpp -lspirv-cross-msl -lspirv-cross-glsl -lspirv-cross-hlsl -lspirv-cross-reflect
LIBS += -L$(brew_prefix)/lib
LIBS += -lglslang -lMachineIndependent -lGenericCodeGen -lOGLCompiler -lOSDependent -lglslang-default-resource-limits -lSPIRV
LIBS += -L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib
LIBS += -lc++

# add all the SPIRV libs
SPIRV_LIBS := $(wildcard external/SPIRV-Cross/build/libspirv*.a)
$(SPIRV_LIBS):                                                                                                                                                                                               	LIBS += $@

GLSL_LIBS := $(wildcard external/glslang/build/glslang/lib*.a)
$(GLSL_LIBS):                                                                                                                                                                                                   LIBS += $@

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
mgl_srcs_c := $(wildcard MGL/src/*.c)
mgl_srcs_objc := $(wildcard MGL/src/*.m)

mgl_objs := $(mgl_srcs_c:.c=.o) $(mgl_srcs_cpp:.cpp=.o)
mgl_objs := $(addprefix $(build_dir)/,$(mgl_objs))

mgl_arc_objs := $(mgl_srcs_objc:.m=.o)
mgl_arc_objs := $(addprefix $(build_dir)/arc/,$(mgl_arc_objs))

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

define index_of
$(info DEBUG: Stripped value of $(1) is: $(strip $(1)))  # Print stripped value
$(foreach dir,$(EXT_DIRS), \
    $(info DEBUG: Checking if $(strip $(1)) matches $(patsubst ./%,%,$(dir)))  # Debug each directory
    $(info DEBUG: filter result: $(filter $(strip $(1)),$(patsubst ./%,%,$(dir))))  # Debug filter result
    $(eval _i:=$(shell echo $$((_i + 1))))  # Increment index
    $(if $(filter $(strip $(1)),$(patsubst ./%,%,$(dir)) ),$(dir))  # If match, print repository
))
endef

# Function to get the corresponding repository URL for a directory
define get_repo_url
$(word $(call index_of,$(1)),$(REPOS))
endef

# Function to check if a directory exists, and if not, clone it
define check_and_clone
	@echo "Resolving directory $(1)..."; \
	INDEX=$(call index_of,$1)); \
	echo "INDEX calculated: $$INDEX"; \
	echo "REPO resolved: $$REPO"; \	if [ ! -d $(1) ]; then \
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
deps += $(mgl_arc_objs:.o=.d)


mgl_lib := $(build_dir)/libmgl.dylib

$(mgl_lib): $(mgl_objs) $(mgl_arc_objs)
	@mkdir -p $(dir $@)
	$(CC) -dynamiclib -o $@ $^ $(LIBS)
	# loading dynamic library requires this
	ln -fs $(mgl_lib) .


# specific rules

#lib: $(build_dir)/libglfw.dylib
lib: $(mgl_lib)

test: $(test_exe)
	$(test_exe)

dbg: $(test_exe)
	lldb -o run $(test_exe)


# generic rules
$(build_dir)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -MMD $(CFLAGS) -c $< -o $@

#-std=gnu17 
$(build_dir)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -MMD $(CFLAGS) -c $< -o $@

#-std=c++14
$(build_dir)/arc/%.o: %.m
	@mkdir -p $(dir $@)
	clang -fobjc-arc -fmodules -MMD $(CFLAGS) -c $< -o $@

$(build_dir)/%.o: %.m
	@mkdir -p $(dir $@)
	clang -fmodules -MMD $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(build_dir)

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
