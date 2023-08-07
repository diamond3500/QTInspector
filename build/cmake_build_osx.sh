#!/usr/bin/env bash
export PATH=$CMAKE311_HOME/bin:/usr/bin:$PATH

IOS_PLATFORM=OS

current_dir=$(cd "$(dirname "$0")";pwd)
echo $current_dir

if [ -z "$COMPILE_ARM" ]; then
    echo 'COMPILE_ARM not defined, set default TRUE'
    COMPILE_ARM=OFF
fi;
# -Dprotobuf_BUILD_PROTOC_BINARIES=1 生成pb 工具
cmake .. -GXcode  -B$current_dir/../project/osx -DIOS_PLATFORM=$IOS_PLATFORM -DCMAKE_PREFIX_PATH=/Users/henry/Qt/5.15.2/clang_64 -DCMAKE_TOOLCHAIN_FILE=$current_dir/toolchain/osx.cmake -DCOMPILE_ARM=$COMPILE_ARM -DBUILD_SHARED_LIBS=0  -DENABLE_LOG_DEBUG=1 -DCMAKE_THREAD_LIBS_INIT=-lpthread
