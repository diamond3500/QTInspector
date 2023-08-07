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
cmake_minimum_required(VERSION 3.24.0)
project(QtInspector)

set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)



if( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
  # warning level 4 and all warnings as errors
  #add_compile_options(/wd4065)
endif()

if(WIN32)
 #set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
 #set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
 
 set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -Od")
 add_definitions(-DOS_WIN)

elseif(ANDROID)
 add_definitions(-DOS_ANDROID)
elseif(APPLE)
    if(IOS)
        add_definitions(-DOS_IOS)
    else()
        add_definitions(-DOS_MACOSX)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OSX Deployment version" FORCE)
    endif()
endif()


add_subdirectory(source)
add_subdirectory(server)
add_subdirectory(example)

