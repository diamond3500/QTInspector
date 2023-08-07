## 环境
### cmake 版本 >= 3.16
### qt 5.12
### mac 系统需要执行 chmod 777 QTInspector/tool/protoc

## 编译
build 目录下有生成脚本，可以生成各个平台下的工程文件。 特别需要注意 每个build 脚本需要自行修改 CMAKE_PREFIX_PATH 变量指向自己的qt 安装目录。这个 CMAKE_PREFIX_PATH 变量bat 或者 sh脚本文件中。
### 1) windows平台。  cmake_build_3264.bat， 生成的工程可以使用VS2019 打开。
### 2) mac平台。 cmake_build_osx.sh， 生成xcode 工程
### 3) 可以直接使用qtcreator 打开工程根目录下 CMakeLists.txt。


## 工程
### QtInspectorCore
静态库，实现inspector 的核心功能，枚举窗口，获取对象元信息，设置属性等。
###  server 
可执行程序， 启动之后监听本地5973端口(tcp)，用来接受客户端的额连接，接收到客户端连接之后，可以查看客户端界面，控件信息，动态修改控件属性，导出客户端资源等。

### example
这个目录下有3个测试工程，分别是qwidget， qml，以及felgo 工程。每个工程可以编译一个可执行程序。 程序启动之后会每隔开1秒连接服务端，连接成功之后会发送 客户端信息，这样就可以在服务端查看和操作。
