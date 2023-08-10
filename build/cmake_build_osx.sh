#!/usr/bin/env bash
IOS_PLATFORM=OS

current_dir=$(cd "$(dirname "$0")";pwd)
echo $current_dir

if [ -z "$COMPILE_ARM" ]; then
    echo 'COMPILE_ARM not defined, set default TRUE'
    COMPILE_ARM=OFF
fi;
# -Dprotobuf_BUILD_PROTOC_BINARIES=1 生成pb 工具
cmake .. -GXcode  -B$current_dir/../project/osx -DIOS_PLATFORM=$IOS_PLATFORM -DCMAKE_PREFIX_PATH=$1 -DCMAKE_TOOLCHAIN_FILE=$current_dir/toolchain/osx.cmake -DCOMPILE_ARM=$COMPILE_ARM -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_PROTOC_BINARIES=OFF -Dprotobuf_INSTALL=OFF
