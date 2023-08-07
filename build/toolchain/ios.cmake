# ﻿cmake_minimum_required(VERSION 3.4.1)

set (CMAKE_SYSTEM_NAME Darwin)
set (UNIX 1)
set (APPLE 1)
set (IOS 1)


# Darwin versions:
#   6.x == Mac OSX 10.2
#   7.x == Mac OSX 10.3
#   8.x == Mac OSX 10.4
#   9.x == Mac OSX 10.5
#  10.x == Mac OSX 10.6 (Snow Leopard)
execute_process(COMMAND sw_vers
  OUTPUT_VARIABLE CMAKE_SYSTEM_VERSION
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)

string (REGEX MATCH "[0-9\\.]+" CMAKE_SYSTEM_VERSION "${CMAKE_SYSTEM_VERSION}")
message (STATUS "Get maxos x version " ${CMAKE_SYSTEM_VERSION})

string (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\1" DARWIN_MAJOR_VERSION "${CMAKE_SYSTEM_VERSION}")
string (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\2" DARWIN_MINOR_VERSION "${CMAKE_SYSTEM_VERSION}")

# Get the Xcode version being used.
execute_process(COMMAND xcodebuild -version
  OUTPUT_VARIABLE XCODE_VERSION
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCH "Xcode [0-9\\.]+" XCODE_VERSION "${XCODE_VERSION}")
string(REGEX REPLACE "Xcode ([0-9\\.]+)" "\\1" XCODE_VERSION "${XCODE_VERSION}")
message(STATUS "Building with Xcode version: ${XCODE_VERSION}")
string (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\1" XCODE_MAJOR_VERSION "${XCODE_VERSION}")
message(STATUS "Building with Xcode major version: ${XCODE_MAJOR_VERSION}")

# Do not use the "-Wl,-search_paths_first" flag with the OSX 10.2 compiler.
# Done this way because it is too early to do a TRY_COMPILE.
if (NOT DEFINED HAVE_FLAG_SEARCH_PATHS_FIRST)
    set (HAVE_FLAG_SEARCH_PATHS_FIRST 0)
    if ("${DARWIN_MAJOR_VERSION}" GREATER 6)
        set (HAVE_FLAG_SEARCH_PATHS_FIRST 1)
    endif ("${DARWIN_MAJOR_VERSION}" GREATER 6)
endif (NOT DEFINED HAVE_FLAG_SEARCH_PATHS_FIRST)
# More desirable, but does not work:
#INCLUDE(CheckCXXCompilerFlag)
#CHECK_CXX_COMPILER_FLAG("-Wl,-search_paths_first" HAVE_FLAG_SEARCH_PATHS_FIRST)

# Force the compilers to gcc for iOS
#include (CMakeForceCompiler)
#CMAKE_FORCE_C_COMPILER (/usr/bin/clang Apple)
#CMAKE_FORCE_CXX_COMPILER (/usr/bin/clang++ Apple)
#set(CMAKE_AR ar CACHE FILEPATH "" FORCE)

# Skip the platform compiler checks for cross compiling
set (CMAKE_CXX_COMPILER_WORKS TRUE)
set (CMAKE_C_COMPILER_WORKS TRUE)

set (CMAKE_SHARED_LIBRARY_PREFIX "lib")
set (CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
set (CMAKE_SHARED_MODULE_PREFIX "lib")
set (CMAKE_SHARED_MODULE_SUFFIX ".so")
set (CMAKE_MODULE_EXISTS 1)
set (CMAKE_DL_LIBS "")


set (CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG "-compatibility_version ")
set (CMAKE_C_OSX_CURRENT_VERSION_FLAG "-current_version ")
set (CMAKE_CXX_OSX_COMPATIBILITY_VERSION_FLAG "${CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG}")
set (CMAKE_CXX_OSX_CURRENT_VERSION_FLAG "${CMAKE_C_OSX_CURRENT_VERSION_FLAG}")


# Additional flags for dynamic framework
if (APPLE_FRAMEWORK AND BUILD_SHARED_LIBS)
  set (CMAKE_MODULE_LINKER_FLAGS "-rpath @executable_path/Frameworks -rpath @loader_path/Frameworks")
  set (CMAKE_SHARED_LINKER_FLAGS "-rpath @executable_path/Frameworks -rpath @loader_path/Frameworks")
  set (CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG 1)
  set (CMAKE_INSTALL_NAME_DIR "@rpath")
endif()

# Hidden visibilty is required for cxx on iOS
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++  ${no_warn}")

set (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -ffast-math")

if (HAVE_FLAG_SEARCH_PATHS_FIRST)
    set (CMAKE_C_LINK_FLAGS "-Wl,-search_paths_first ${CMAKE_C_LINK_FLAGS}")
    set (CMAKE_CXX_LINK_FLAGS "-Wl,-search_paths_first ${CMAKE_CXX_LINK_FLAGS}")
endif (HAVE_FLAG_SEARCH_PATHS_FIRST)

set (CMAKE_PLATFORM_HAS_INSTALLNAME 1)
set (CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-dynamiclib -headerpad_max_install_names")
set (CMAKE_SHARED_MODULE_CREATE_C_FLAGS "-bundle -headerpad_max_install_names")
set (CMAKE_SHARED_MODULE_LOADER_C_FLAG "-Wl,-bundle_loader,")
set (CMAKE_SHARED_MODULE_LOADER_CXX_FLAG "-Wl,-bundle_loader,")
set (CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".so" ".a")

# hack: if a new cmake (which uses CMAKE_INSTALL_NAME_TOOL) runs on an old build tree
# (where install_name_tool was hardcoded) and where CMAKE_INSTALL_NAME_TOOL isn't in the cache
# and still cmake didn't fail in CMakeFindBinUtils.cmake (because it isn't rerun)
# hardcode CMAKE_INSTALL_NAME_TOOL here to install_name_tool, so it behaves as it did before, Alex
if (NOT DEFINED CMAKE_INSTALL_NAME_TOOL)
    find_program(CMAKE_INSTALL_NAME_TOOL install_name_tool)
endif (NOT DEFINED CMAKE_INSTALL_NAME_TOOL)


# Setup iOS platform unless specified manually with IOS_PLATFORM
if (NOT DEFINED IOS_PLATFORM)
	set (IOS_PLATFORM "OS")
  set (IPHONEOS ON)
  set(XCODE_IOS_PLATFORM iphoneos)
  message (STATUS "Does not defined IOS_PLATFORM , use default IOS_PLATFORM=OS")
elseif (${IOS_PLATFORM} STREQUAL "OS")
  message (STATUS "Use defined IOS_PLATFORM " ${IOS_PLATFORM})
  set (IPHONEOS ON)
  set(XCODE_IOS_PLATFORM iphoneos)
elseif (${IOS_PLATFORM} STREQUAL "SIMULATOR")
    set (IPHONESIMULATOR ON)
    set(XCODE_IOS_PLATFORM iphonesimulator)
    #xcode9 simulator forced to arch x86_64
    if(${XCODE_MAJOR_VERSION} LESS 9)
        message (STATUS "Use defined IOS_PLATFORM " ${IOS_PLATFORM})
        set(IOS_ARCH i386 CACHE STRING "")
    else()
        set (IPHONESIMULATOR64 ON)
        set(IOS_ARCH x86_64 CACHE STRING "")
        message (STATUS "Xcode ${XCODE_VERSION} forced simulator to x86_64 , Using simulator64")
    endif()
elseif(IOS_PLATFORM STREQUAL "SIMULATOR64")
    set (IPHONESIMULATOR ON)
    set (IPHONESIMULATOR64 ON)
    set(XCODE_IOS_PLATFORM iphonesimulator)
    set(IOS_ARCH x86_64 CACHE STRING "")
    message (STATUS "Using simulator64")
else()
  message (FATAL_ERROR "Unsupported IOS_PLATFORM value selected. Please choose OS or SIMULATOR")
endif (NOT DEFINED IOS_PLATFORM)

set (IOS_PLATFORM ${IOS_PLATFORM} CACHE STRING "Type of iOS Platform")

# Setup architecture
if (NOT DEFINED IOS_ARCH)
     set (IOS_ARCH arm64 armv7 CACHE STRING "")
     message (STATUS "Does not defined IOS_ARCH , use default arm64 armv7")
else ()
    message (STATUS "Use defined IOS_ARCH " ${IOS_ARCH})
endif (NOT DEFINED IOS_ARCH)
set (CMAKE_OSX_ARCHITECTURES ${IOS_ARCH} CACHE string "Build architecture for iOS")


# If user did not specify the SDK root to use, then query xcodebuild for it.
if (NOT CMAKE_OSX_SYSROOT)
  execute_process(COMMAND xcodebuild -version -sdk ${XCODE_IOS_PLATFORM} Path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Not defined CMAKE_OSX_SYSROOT , Using SDK: ${CMAKE_OSX_SYSROOT} for platform: ${IOS_PLATFORM}")
endif()

if (NOT EXISTS ${CMAKE_OSX_SYSROOT})
  message(FATAL_ERROR "Invalid CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT} "
    "does not exist.")
endif()
message (STATUS "Using CMAKE_OSX_SYSROOT " ${CMAKE_OSX_SYSROOT})

# Get the SDK version information.
execute_process(COMMAND xcodebuild -sdk ${CMAKE_OSX_SYSROOT} -version SDKVersion
  OUTPUT_VARIABLE IOS_SDK_VERSION
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)
# Find the Developer root for the specific iOS platform being compiled for
# from CMAKE_OSX_SYSROOT.  Should be ../../ from SDK specified in
# CMAKE_OSX_SYSROOT.  There does not appear to be a direct way to obtain
# this information from xcrun or xcodebuild.
if (NOT CMAKE_IOS_DEVELOPER_ROOT)
  get_filename_component(IOS_PLATFORM_SDK_DIR ${CMAKE_OSX_SYSROOT} PATH)
  get_filename_component(CMAKE_IOS_DEVELOPER_ROOT ${IOS_PLATFORM_SDK_DIR} PATH)
endif()
if (NOT EXISTS ${CMAKE_IOS_DEVELOPER_ROOT})
  message(FATAL_ERROR "Invalid CMAKE_IOS_DEVELOPER_ROOT: "
    "${CMAKE_IOS_DEVELOPER_ROOT} does not exist.")
endif()

set(CMAKE_SYSTEM_VERSION ${IOS_SDK_VERSION})

set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "9.0" CACHE STRING "Deployment target for ios" FORCE)
set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO" CACHE STRING "Disbale Bitcode" FORCE)
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH ${ACTIVE_ARCH_ONLY} CACHE STRING "build only active arch" FORCE)
message(STATUS "ACTIVE ARCH ONLY=" ${ACTIVE_ARCH_ONLY})

# Find the C & C++ compilers for the specified SDK.
if (NOT CMAKE_C_COMPILER)
  execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find clang
    OUTPUT_VARIABLE CMAKE_C_COMPILER
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Using C compiler: ${CMAKE_C_COMPILER}")
endif()
if (NOT CMAKE_CXX_COMPILER)
  execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find clang++
    OUTPUT_VARIABLE CMAKE_CXX_COMPILER
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Using CXX compiler: ${CMAKE_CXX_COMPILER}")
endif()

# Find (Apple's) libtool.
execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find libtool
  OUTPUT_VARIABLE IOS_LIBTOOL
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Using libtool: ${IOS_LIBTOOL}")
# Configure libtool to be used instead of ar + ranlib to build static libraries.
# This is required on Xcode 7+, but should also work on previous versions of
# Xcode.
set(CMAKE_C_CREATE_STATIC_LIBRARY
  "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")
set(CMAKE_CXX_CREATE_STATIC_LIBRARY
  "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")


# Check the platform selection and setup for developer root
if (IPHONEOS)
	set (IOS_PLATFORM_LOCATION "iPhoneOS.platform")

	# This causes the installers to properly locate the output libraries
	set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos")
  #set (CMAKE_OSX_SYSROOT "iphoneos")
elseif (IPHONESIMULATOR)
	set (IOS_PLATFORM_LOCATION "iPhoneSimulator.platform")

	# This causes the installers to properly locate the output libraries
	set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphonesimulator")
  #set (CMAKE_OSX_SYSROOT "iphonesimulator")
endif (IPHONEOS)

# shuold set fllow two flags , or you will received error
# like 'target specifies product type 'com.apple.product-type.tool', but there's no such product type for the 'iphoneos' platform
# see issues 'https://cmake.org/Bug/view.php?id=15329
set (CMAKE_MACOSX_BUNDLE YES)
set (CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
set (CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
#set(CMAKE_OSX_DEPLOYMENT_TARGET 7.0)

# Setup iOS developer location unless specified manually with CMAKE_IOS_DEVELOPER_ROOT
# Note Xcode 4.3 changed the installation location, choose the most recent one available
set (XCODE_POST_43_ROOT "/Applications/Xcode.app/Contents/Developer/Platforms/${IOS_PLATFORM_LOCATION}/Developer")
set (XCODE_PRE_43_ROOT "/Developer/Platforms/${IOS_PLATFORM_LOCATION}/Developer")
if (NOT DEFINED CMAKE_IOS_DEVELOPER_ROOT)
	if (EXISTS ${XCODE_POST_43_ROOT})
		set (CMAKE_IOS_DEVELOPER_ROOT ${XCODE_POST_43_ROOT})
    message (STATUS "Not defined CMAKE_IOS_DEVELOPER_ROOT , use " ${CMAKE_IOS_DEVELOPER_ROOT})
	elseif(EXISTS ${XCODE_PRE_43_ROOT})
		set (CMAKE_IOS_DEVELOPER_ROOT ${XCODE_PRE_43_ROOT})
    message (STATUS "Not defined CMAKE_IOS_DEVELOPER_ROOT , use " ${CMAKE_IOS_DEVELOPER_ROOT})
	endif (EXISTS ${XCODE_POST_43_ROOT})
else()
  message (STATUS "Use defined CMAKE_IOS_DEVELOPER_ROOT " ${CMAKE_IOS_DEVELOPER_ROOT})
endif (NOT DEFINED CMAKE_IOS_DEVELOPER_ROOT)
set (CMAKE_IOS_DEVELOPER_ROOT ${CMAKE_IOS_DEVELOPER_ROOT} CACHE PATH "Location of iOS Platform")

# Find and use the most recent iOS sdk unless specified manually with CMAKE_IOS_SDK_ROOT
if (NOT DEFINED CMAKE_IOS_SDK_ROOT)
	file (GLOB _CMAKE_IOS_SDKS "${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/*")
	if (_CMAKE_IOS_SDKS)
		list (SORT _CMAKE_IOS_SDKS)
		list (REVERSE _CMAKE_IOS_SDKS)
		list (GET _CMAKE_IOS_SDKS 0 CMAKE_IOS_SDK_ROOT)
	else (_CMAKE_IOS_SDKS)
		message (FATAL_ERROR "No iOS SDK's found in default search path ${CMAKE_IOS_DEVELOPER_ROOT}. Manually set CMAKE_IOS_SDK_ROOT or install the iOS SDK.")
	endif (_CMAKE_IOS_SDKS)
	message (STATUS "Toolchain using default iOS SDK: ${CMAKE_IOS_SDK_ROOT}")
endif (NOT DEFINED CMAKE_IOS_SDK_ROOT)
set (CMAKE_IOS_SDK_ROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Location of the selected iOS SDK")


# Set the find root to the iOS developer roots and to user defined paths
set (CMAKE_FIND_ROOT_PATH ${CMAKE_IOS_DEVELOPER_ROOT} ${CMAKE_IOS_SDK_ROOT} ${CMAKE_PREFIX_PATH} CACHE string  "iOS find search path root")

# default to searching for frameworks first
set (CMAKE_FIND_FRAMEWORK FIRST)

# set up the default search directories for frameworks
set (CMAKE_SYSTEM_FRAMEWORK_PATH
    ${CMAKE_IOS_SDK_ROOT}/System/Library/Frameworks
    ${CMAKE_IOS_SDK_ROOT}/System/Library/PrivateFrameworks
    ${CMAKE_IOS_SDK_ROOT}/Developer/Library/Frameworks
)

if ("${CMAKE_BACKWARDS_COMPATIBILITY}" MATCHES "^1\\.[0-6]$")
    set (CMAKE_SHARED_MODULE_CREATE_C_FLAGS "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -flat_namespace -undefined suppress")
endif ("${CMAKE_BACKWARDS_COMPATIBILITY}" MATCHES "^1\\.[0-6]$")

if (NOT XCODE)
      # Enable shared library versioning.  This flag is not actually referenced
      # but the fact that the setting exists will cause the generators to support
      # soname computation.
      set (CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-install_name")
endif (NOT XCODE)

# only search the iOS sdks, not the remainder of the host filesystem
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


macro (find_osx_framework VAR fwname)
    find_library(FRAMEWORK_${fwname}
                 NAMES ${fwname}
                 PATHS ${CMAKE_OSX_SYSROOT}/System/Library
                 PATH_SUFFIXES Frameworks
                 NO_DEFAULT_PATH)

    if(${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
        message(ERROR "Framework ${fwname} not found")
    else()
        message(STATUS "Framwork ${fwname} found at ${FRAMEWORK_${fwname}}")
        set(${VAR} ${FRAMEWORK_${fwname}})
    endif()

endmacro(find_osx_framework)

macro (set_xcode_attr_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
endmacro (set_xcode_attr_property)

macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property (TARGET ${TARGET} PROPERTY ${XCODE_PROPERTY} ${XCODE_VALUE})
endmacro (set_xcode_property)
