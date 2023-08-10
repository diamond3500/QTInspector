#!/usr/bin/env bash
export PATH=$CMAKE311_HOME/bin:/usr/bin:$PATH

current_dir=$(cd "$(dirname "$0")";pwd)
echo $current_dir

IOS_PLATFORM=OS
IOS_PLATFORM_SIMULATOR=SIMULATOR
cmake .. -GXcode -B$current_dir/../project/ios  -DIOS_PLATFORM=$IOS_PLATFORM_SIMULATOR -DCMAKE_PREFIX_PATH=$1 -DCMAKE_TOOLCHAIN_FILE=$current_dir/toolchain/ios.cmake -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_PROTOC_BINARIES=OFF -Dprotobuf_INSTALL=OFF