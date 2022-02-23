# first, with brew installed, do 'make install-pkgdeps'
# then configure here

# Find SDK path via xcode-select, backwards compatible with Xcode vers < 4.5
# on M1 monterey, comment out the following line
SDK_ROOT = $(shell xcode-select -p)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.1.sdk

# with installed spirv_headers and spirv_cross
# spirv_headers_include_path := /usr/local/include
# spirv_cross_include_path := /usr/local/include/spirv
# spirv_cross_1_2_include_path := /usr/local/include/spirv/1.2
# spirv_cross_config_include_path := external/SPIRV-Cross
# spirv_cross_lib_path := /usr/local/lib

# with uninstalled spirv_headers and spirv_cross
# uncomment the following lines 
# spirv_headers_path := ../SPIRV-Headers/include/spirv/1.2
# spirv_cross_include_path := ../SPIRV-Cross
# spirv_cross_lib_path := ../SPIRV-Cross/build

# build dir
build_dir := build
