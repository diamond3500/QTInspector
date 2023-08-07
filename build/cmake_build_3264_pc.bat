@echo off


cmake .. -G"Visual Studio 16 2019" -B../project/win32_pc -DCMAKE_CONFIGURATION_TYPES=Debug;Release -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_PROTOC_BINARIES=OFF -Dprotobuf_INSTALL=OFF -DCMAKE_PREFIX_PATH=C:/Felgo/Felgo/msvc2019_64 
pause
@echo on
