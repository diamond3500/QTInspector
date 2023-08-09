#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements. See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership. The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the
# specific language governing permissions and limitations
# under the License.
#
cmake_minimum_required(VERSION 3.16.0)
set(PROJECT_NAME QtInspectorCore)
project(${PROJECT_NAME} LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Qml Quick)

FILE (GLOB sources *.cc *.h)
SOURCE_GROUP("sources" FILES ${sources})


IF (CMAKE_SYSTEM_NAME MATCHES "Windows") 
    MESSAGE(STATUS "current platform: Windows") 
    set (ProtoCommandPath ${CMAKE_CURRENT_SOURCE_DIR}/../tool/protoc.exe)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin") 
    MESSAGE(STATUS "current platform: Mac OS X") 
    set (ProtoCommandPath ${CMAKE_CURRENT_SOURCE_DIR}/../tool/protoc)
ELSE () 
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}") 
ENDIF () 
 #协议文件夹
 set(ProtocolDir "")
 cmake_path(APPEND_STRING ProtocolDir "${CMAKE_CURRENT_SOURCE_DIR}/pb")
 #协议文件生成的临时目录
 set(outdir "${CMAKE_CURRENT_SOURCE_DIR}/../out/proto")
 #删除现在的目录
 file(REMOVE_RECURSE ${outdir})
#递归遍历 模块文件夹下的pb 文件
file(GLOB_RECURSE protos ${ProtocolDir}/*.proto)
foreach(protoPath IN LISTS protos)
    if (NOT EXISTS ${outdir})
        file (MAKE_DIRECTORY ${outdir})
    endif ()

    #生成c++ 使用的pb 头文件和cc 文件
    set(cppout "")
    cmake_path(APPEND_STRING cppout "--cpp_out=" "${outdir}")

    get_filename_component(protoName ${protoPath} NAME)
    execute_process(COMMAND ${ProtoCommandPath} "-I=${ProtocolDir}" ${cppout} ${protoName})
endforeach()

 #拷贝c++ 到目标目录
 file(GLOB_RECURSE generateFiles ${outdir}/*.h ${outdir}/*.cc)
 foreach(generateFile IN LISTS generateFiles)
     execute_process(COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" ${generateFile} ${ProtocolDir})
 endforeach()


FILE (GLOB pb pb/*.cc pb/*.h pb/*.proto)
SOURCE_GROUP("pb" FILES ${pb})

FILE (GLOB util util/*.cc util/*.h)
SOURCE_GROUP("util" FILES ${util})

FILE (GLOB network network/*.cc network/*.h)
SOURCE_GROUP("network" FILES ${network})

add_library(${PROJECT_NAME} ${sources} ${pb} ${util} ${network})

message("Qt version: ${Qt${QT_VERSION_MAJOR}_VERSION_MAJOR}.${Qt${QT_VERSION_MAJOR}_VERSION_MINOR}.${Qt${QT_VERSION_MAJOR}_VERSION_PATCH}")

target_compile_options(${PROJECT_NAME} PRIVATE /W4)
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_FLAGS "/wd4100 /wd4996")
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_include_directories(${PROJECT_NAME} PRIVATE ${QT_PRIVATE_INCLUDES})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Qml Qt${QT_VERSION_MAJOR}::Quick Qt${QT_VERSION_MAJOR}::Widgets libprotobuf-lite libprotobuf Ws2_32.lib)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Qml Qt${QT_VERSION_MAJOR}::Quick Qt${QT_VERSION_MAJOR}::Widgets libprotobuf-lite libprotobuf)
endif()

add_subdirectory(3rd)
