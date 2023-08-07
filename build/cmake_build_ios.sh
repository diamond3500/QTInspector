#!/usr/bin/env bash
export PATH=$CMAKE311_HOME/bin:/usr/bin:$PATH

current_dir=$(cd "$(dirname "$0")";pwd)
echo $current_dir

IOS_PLATFORM=OS
IOS_PLATFORM_SIMULATOR=SIMULATOR
cmake ..  -GXcode -B$current_dir/../project/ios  -DIOS_PLATFORM=$IOS_PLATFORM_SIMULATOR -DGO_EXECUTABLE=/usr/local/go/bin/go -DPERL_EXECUTABLE=/usr/local/bin/perl -DCMAKE_PREFIX_PATH=/Users/henry/Qt/5.15.2/ios -DCMAKE_TOOLCHAIN_FILE=$current_dir/toolchain/ios.cmake -DBUILD_SHARED_LIBS=0 -DENABLE_LOG_DEBUG=1 -DCMAKE_THREAD_LIBS_INIT=-lpthread


# IOS_PLATFORM_SIMULATOR=SIMULATOR
 #cmake .. -GXcode -B$current_dir/../project/ios_simulator -DIOS_PLATFORM=$IOS_PLATFORM_SIMULATOR -DCMAKE_TOOLCHAIN_FILE=$current_dir/toolchain/ios.cmake -DBUILD_SHARED_LIBS=0 -DBOOST_EXCLUDE_LIBRARIES=test -DENABLE_LOG_DEBUG=1 -DLEVELDB_BUILD_TESTS=OFF -DLEVELDB_BUILD_BENCHMARKS=OFF -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_PROTOC_BINARIES=OFF -Dprotobuf_INSTALL=OFF -DCMAKE_THREAD_LIBS_INIT=-lpthread

