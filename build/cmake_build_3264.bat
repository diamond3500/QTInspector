@echo off


cmake .. -G"Visual Studio 16 2019" -B../project/win32 -DCMAKE_CONFIGURATION_TYPES=Debug;Release -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_PROTOC_BINARIES=OFF -Dprotobuf_INSTALL=OFF -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_PREFIX_PATH=%1
pause
@echo on
