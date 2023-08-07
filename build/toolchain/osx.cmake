# ﻿cmake_minimum_required(VERSION 3.4.1)

set (CMAKE_SYSTEM_NAME Darwin)
set (UNIX 1)
set (APPLE 1)
set (OSX 1)

if((NOT DEFINED COMPILE_ARM) OR (COMPILE_ARM STREQUAL "")
    OR (${COMPILE_ARM} MATCHES "(FALSE|false|0|OFF)"))
  message(STATUS "option COMPILE_ARM set OFF")
  option(COMPILE_ARM "" OFF)
elseif(${COMPILE_ARM} MATCHES "(TRUE|true|1|null|ON)")
  message(STATUS "option COMPILE_ARM set ON")
  option(COMPILE_ARM "" ON)
else()
  message(FATAL_ERROR "INVALID FLAG COMPILE_ARM=" ${COMPILE_ARM})
endif()

# Get the Xcode version being used.
execute_process(COMMAND xcodebuild -version
  OUTPUT_VARIABLE XCODE_VERSION
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCH "Xcode [0-9\\.]+" XCODE_VERSION "${XCODE_VERSION}")
string(REGEX REPLACE "Xcode ([0-9\\.]+)" "\\1" XCODE_VERSION "${XCODE_VERSION}")
string (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\1" XCODE_MAJOR_VERSION "${XCODE_VERSION}")
string (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\2" XCODE_MINOR_VERSION "${XCODE_VERSION}")
message(STATUS "Building with Xcode version: ${XCODE_VERSION}")


if (COMPILE_ARM AND (${XCODE_MAJOR_VERSION} GREATER 12 OR ((${XCODE_MAJOR_VERSION} EQUAL 12) AND (((${XCODE_MINOR_VERSION} EQUAL 2) OR (${XCODE_MINOR_VERSION} GREATER 2))))))
  set(OSX_ARCH x86_64 arm64 CACHE STRING "")
else()
  set(OSX_ARCH x86_64 CACHE STRING "")
endif()
message(STATUS "OSX_ARCH: ${OSX_ARCH}")


set(CMAKE_XCODE_ATTRIBUTE_CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Deployment target for osx" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Deployment target for osx" FORCE)
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
# apple use 'c++0x' instead of 'c++11'
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
set(CMAKE_OSX_ARCHITECTURES ${OSX_ARCH} CACHE STRING "Build architecture for osx")
set(CMAKE_XCODE_ARCHS ${OSX_ARCH} CACHE STRING "Build architecture for osx")

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






