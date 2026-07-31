# cmake基础命令收集整理

## 1. 常用指令

reference:

1、 https://zhuanlan.zhihu.com/p/406559772

2、https://zhuanlan.zhihu.com/p/534439206

3、[csdn: add_custom_target](https://blog.csdn.net/MacKendy/article/details/122693478)

4、https://github.com/wzpan/cmake-demo/tree/master

### 1.1 指定 cmake 的最小版本

```cmake
cmake_minimum_required(VERSION 3.4.1 REQUIRED)
```

### 1.2 设置项目名称

```cmake
project(demo)

# 它会引入两个变量 demo_BINARY_DIR 和 demo_SOURCE_DIR，同时，cmake 自动定义了两个等价PROJECT_BINARY_DIR 和 PROJECT_SOURCE_DIR.
```

### 1.3 设置编译类型

```cmake
# add_library 默认生成是静态库

add_executable(demo demo.cpp) # 生成可执行文件
add_library(common STATIC util.cpp) # 生成静态库
add_library(common SHARED util.cpp) # 生成动态库或共享库

以上命令将生成：
在 Linux 下是：
        demo
        libcommon.a
        libcommon.so
在 Windows 下是：
        demo.exe
        common.lib
        common.dll
```

### 1.4 明确指定包含哪些源文件

```cmake
add_library(demo demo.cpp test.cpp util.cpp)
```

### 1.5 设置变量

```cmake
# set 直接设置变量的值
set(SRC_LIST main.cpp test.cpp)
add_executable(demo ${SRC_LIST})
set(ROOT_DIR ${CMAKE_SOURCE_DIR}) #CMAKE_SOURCE_DIR默认为当前cmakelist.txt目录

# set追加设置变量的值
set(SRC_LIST main.cpp)
set(SRC_LIST ${SRC_LIST} test.cpp)
add_executable(demo ${SRC_LIST})

# set缓存变量的值：set(var value CACHE type[PATH|BOOL|STRING|] docstring)
set(CMAKE_INSTALL_PREFIX "/path/to/install" CACHE PATH "The install prefix location.")

# 总结：
# set(var value) 仅在内存中设置变量，不会持久化。
# set(var value CACHE type docstring) 不仅设置变量值，还将其添加到缓存中，使其成为可配置项，支持持久化存储和交互式修改。
# 在需要用户自定义或跨构建保留配置信息时，使用带有CACHE的set命令更为合适。

# 特别说明：
# 在交叉编译工具链cmake文件中指定CMAKE_INSTALL_PREFIX，需要使用CACHE PATH，否则该变量可能不会生效；
# 这种情况需要在cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake_arm_toolchain.cmake -DCMAKE_INSTALL_PREFIX=/path/to/install
# 这样设置的变量具有较高的优先级。

# list追加或者删除变量的值
set(SRC_LIST main.cpp)
list(APPEND SRC_LIST test.cpp)
list(REMOVE_ITEM SRC_LIST main.cpp)
add_executable(demo ${SRC_LIST})
```

### 1.6 搜索文件

```cmake
# 搜索当前目录下的所有.cpp文件，并命名为SRC_LIST，它会查找目录下的.c,.cpp ,.mm,.cc 等等C/C++语言后缀的文件名
aux_source_directory(. SRC_LIST) 
add_library(demo ${SRC_LIST})

# 自定义搜索规则
aux_source_directory(. SRC_LIST)
aux_source_directory(protocol SRC_PROTOCOL_LIST)
add_library(demo ${SRC_LIST} ${SRC_PROTOCOL_LIST})
# 或者
file(GLOB SRC_LIST "*.cpp" "protocol/*.cpp")
add_library(demo ${SRC_LIST})
# 或者
file(GLOB SRC_LIST "*.cpp")
file(GLOB SRC_PROTOCOL_LIST "protocol/*.cpp")
add_library(demo ${SRC_LIST} ${SRC_PROTOCOL_LIST})
# 或者
file(GLOB_RECURSE SRC_LIST "*.cpp") #递归搜索
FILE(GLOB SRC_PROTOCOL RELATIVE "protocol" "*.cpp") # 相对protocol目录下搜索
add_library(demo ${SRC_LIST} ${SRC_PROTOCOL_LIST})
```

### 1.7 设置包含的目录，头文件目录

```cmake
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# Linux 下还可以通过如下方式设置包含的目录
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}")
```

### 1.8 设置链接库搜索目录

```cmake
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)
# Linux 下还可以通过如下方式设置包含的目录
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_CURRENT_SOURCE_DIR}/libs")
```

### 1.9 设置 target 需要链接的库

```cmake
# 指定链接动态库或静态库
target_link_libraries(demo libface.a) # 链接libface.a
target_link_libraries(demo libface.so) # 链接libface.so

# 指定全路径
target_link_libraries(demo ${CMAKE_CURRENT_SOURCE_DIR}/libs/libface.a)
target_link_libraries(demo ${CMAKE_CURRENT_SOURCE_DIR}/libs/libface.so)

# 指定链接多个库
target_link_libraries(demo
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libface.a
    boost_system.a
    boost_thread
    pthread)
 
# 注意：target_link_libraries必须在add_excutable或者add_library之后，否则会出现cmake编译错误
```

### 1.10 打印信息

```cmake
message(${PROJECT_SOURCE_DIR})
message("build with debug mode")
message(WARNING "this is warnning message")
message(FATAL_ERROR "this build has many error") # FATAL_ERROR 会导致编译失败
```

### 1.11 包含其它 cmake 文件

```cmake
include(./common.cmake) # 指定包含文件的全路径
include(def) # 在搜索路径中搜索def.cmake文件
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake) # 设置include的搜索路径
```

### 1.12 条件控制

```cmake
# if…elseif…else…endif
# 逻辑判断和比较：
if (expression)：expression 不为空（0,N,NO,OFF,FALSE,NOTFOUND）时为真
if (not exp)：与上面相反
if (var1 AND var2)
if (var1 OR var2)
if (COMMAND cmd)：如果 cmd 确实是命令并可调用为真
if (EXISTS dir) if (EXISTS file)：如果目录或文件存在为真
if (file1 IS_NEWER_THAN file2)：当 file1 比 file2 新，或 file1/file2 中有一个不存在时为真，文件名需使用全路径
if (IS_DIRECTORY dir)：当 dir 是目录时为真
if (DEFINED var)：如果变量被定义为真
if (var MATCHES regex)：给定的变量或者字符串能够匹配正则表达式 regex 时为真，此处 var 可以用 var 名，也可以用 ${var}
if (string MATCHES regex)

# 数字比较：
if (variable LESS number)：LESS 小于
if (string LESS number)
if (variable GREATER number)：GREATER 大于
if (string GREATER number)
if (variable EQUAL number)：EQUAL 等于
if (string EQUAL number)

# 字母表顺序比较：
if (variable STRLESS string)
if (string STRLESS string)
if (variable STRGREATER string)
if (string STRGREATER string)
if (variable STREQUAL string)
if (string STREQUAL string)

12.2 while…endwhile
12.3 foreach…endforeach
foreach(i RANGE 1 9 2)
    message(${i})
endforeach(i)
# 输出：13579
```

### 1.13 常用变量

```cmake
# 预定义变量
PROJECT_SOURCE_DIR：工程的根目录
PROJECT_BINARY_DIR：运行 cmake 命令的目录，通常是 ${PROJECT_SOURCE_DIR}/build
PROJECT_NAME：返回通过 project 命令定义的项目名称
CMAKE_CURRENT_SOURCE_DIR：当前处理的 CMakeLists.txt 所在的路径
CMAKE_CURRENT_BINARY_DIR：target 编译目录
CMAKE_CURRENT_LIST_DIR：CMakeLists.txt 的完整路径
CMAKE_CURRENT_LIST_LINE：当前所在的行
CMAKE_MODULE_PATH：定义自己的 cmake 模块所在的路径，SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)，然后可以用INCLUDE命令来调用自己的模块
~~EXECUTABLE_OUTPUT_PATH：重新定义目标二进制可执行文件的存放位置(**已被废弃并不建议使用**)~~
~~LIBRARY_OUTPUT_PATH：重新定义目标链接库文件的存放位置(**已被废弃并不建议使用**)~~
CMAKE_SOURCE_DIR: 顶层 CMakeLists.txt 所在目录
CMAKE_RUNTIME_OUTPUT_DIRECTORY: 可执行文件的输出目录（全局）
RUNTIME_OUTPUT_DIRECTORY: 可执行文件的输出目录
CMAKE_LIBRARY_OUTPUT_DIRECTORY: lib库的输出目录（全局）
LIBRARY_OUTPUT_DIRECTORY：lib库的输出目录

# 环境变量
$ENV{Name}
set(ENV{Name} value) # 这里没有“$”符号

# 系统信息
CMAKE_MAJOR_VERSION：cmake 主版本号，比如 3.4.1 中的 3
CMAKE_MINOR_VERSION：cmake 次版本号，比如 3.4.1 中的 4
CMAKE_PATCH_VERSION：cmake 补丁等级，比如 3.4.1 中的 1
CMAKE_SYSTEM：系统名称，比如 Linux-2.6.22
CMAKE_SYSTEM_NAME：不包含版本的系统名，比如 Linux
CMAKE_SYSTEM_VERSION：系统版本，比如 2.6.22
CMAKE_SYSTEM_PROCESSOR：处理器名称，比如 i686
UNIX：在所有的类 UNIX 平台下该值为 TRUE，包括 OS X 和 cygwin
WIN32：在所有的 win32 平台下该值为 TRUE，包括 cygwin
```

### 1.14 主要开关选项

```cmake
BUILD_SHARED_LIBS：这个开关用来控制默认的库编译方式，如果不进行设置，使用 add_library 又没有指定库类型的情况下，默认编译生成的库都是静态库。如果 set(BUILD_SHARED_LIBS ON) 后，默认生成的为动态库
CMAKE_C_FLAGS：设置 C 编译选项，也可以通过指令 add_definitions() 添加
CMAKE_CXX_FLAGS：设置 C++ 编译选项，也可以通过指令 add_definitions() 添加
add_definitions(-DENABLE_DEBUG -DABC) # 参数之间用空格分隔
```

### 1.15 install

```cmake
# 指定 MathFunctions 库的安装路径
install (TARGETS MathFunctions DESTINATION bin)
install (FILES MathFunctions.h DESTINATION include)

# 指定install安装目录
# linux下默认为路劲：/usr/local/bin、/usr/local/include、/usr/local/lib
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)

# 注：默认install之后会去掉`runpath`属性；可以使用以下方式设置动态库的rpath；详情请参考[linux动态库链接说明](./linux动态库链接说明.md)
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
```

### 1.16 add_custom_target

```cmake
# 添加cmake执行命令
add_custom_target(
    print-pwd
    ALL # 编译时就会执行，缺少该关键字，需要执行make print-pwd才会生效
    COMMAND "pwd"
)

# 添加依赖关系depend-print-pwd依赖print-pwd命令
add_custom_target(
    depend-print-pwd
    COMMAND "echo" "depend pwd" # 注意command参数使用空格进行传递，不能使用 "echo 'depend pwd'"
    DEPENDS print-pwd
)

# 指定command命令的所在目录（支持绝对路径和相对路径）
add_custom_target(
    work-dir
    COMMAND "pwd"
    WORKING_DIRECTORY "/root/lemon/cmake-test/build" # 绝对路径 or 相对路径
)
```



### 1.17 cpack打包

```cmake
# 打包：cmake编译完成之后执行以下命令
# 1. cpack -C CPackConfig.cmake：生成二进制安装包
# 2. cpack -C CPackSourceConfig.cmake：生成源码安装包
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "1")
include (CPack)
```

```cmake
# 指定打包格式为 **ZIP**（CPack 支持 ZIP、NSIS、DEB、RPM 等多种生成器）
set(CPACK_GENERATOR "ZIP")

# 包名设为 `Test`
set(CPACK_PACKAGE_NAME "Test")

# 输出文件名固定为 `Test.zip`（默认会是 `Test-<version>-<platform>.zip`，这里强制去掉了版本和平台后缀）。
set(CPACK_PACKAGE_FILE_NAME "Test")

# zip 文件输出到 **build 目录的上一级**（即项目根目录），这样 CI 脚本里 `mv Test.zip packages` 就能直接找到。
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_BINARY_DIR}/..")

# 这是关键——告诉 CPack **把什么文件装进包里**：
# - **源目录**：`bin/`
# - **目标位置**：`.`（zip 包内的根目录）
# - 效果：把该目录下所有文件（exe、dll、配置文件等）**原样平铺**打进 zip
install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/" DESTINATION . )

# 启用 CPack，加载 CPack 模块，注册 `package` 构建目标。
include(CPack)

# 之后执行以下命令，就会生成 `Test.zip`。
cmake --build build --target package
```

**整体流程**
```
CMake Configure → cmake --build → cmake --build --target package
                                        ↓
		                            收集 bin/* 
                                        ↓
                              CPack 打包 → Test.zip（输出到项目根目录）
```

### 1.18 cpp编译选项

```cmake
# 设置c++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 设置c++标准
add_compile_options(-std=c++11)

# 设置类似于生成pdb符号的编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
```



### 1.19 FetchContent

如果cmake中使用FetchContent：`GetJsonStruct.cmake`

```cmake
include(FetchContent)
message("Fetching json_struct...")

set(json_struct_DOWNLOAD_URL
    "https://github.com/jorgen/json_struct/archive/refs/tags/1.0.1.zip"
    CACHE STRING "")

FetchContent_Declare(
    json_struct
    URL ${json_struct_DOWNLOAD_URL}
)

FetchContent_GetProperties(json_struct)
if(NOT json_struct_POPULATED)
    FetchContent_MakeAvailable(json_struct)
endif()

# target: json_struct::json_struct
```

但是资源下载不了，可以手动下载后之后指定FETCHCONTENT_BASE_DIR来避免在cmake运行时再去下载

```sh
# CMakeLists.txt中引用GetJsonStruct.cmake去github下载资源
include(GetJsonStruct)

cmake .. -DFETCHCONTENT_BASE_DIR=../third_party/
```



## 2. cmakelists.txt示例

```cmake
# 指定cmake版本
cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

# 指定项目
project(Test)

# 语言标准
add_compile_options(-std=c++17)

# 编译类型：Debug|Release
if(not CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 编译平台：WINDOWS|LINUX
if(WIN32)
    add_definitions(-DWINDOWS)
else()
    add_definitions(-DLINUX)
endif()

# 系统位数
# if(ARCH_M32)
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
# else()
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
# endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}, Platform: ${CMAKE_SYSTEM_NAME}")

# 编译单元测试选项
option(BUILD_TESTING "build tests or not" OFF)

# 设置包含目录
include_directories(third_party/openssl
                    third_party/libevent
                    third_party/thrift
                    third_party/doctest)

# 链接库路径
if(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        link_directories(target/x64-windows/lib/debug/lib)
    else()
        link_directories(target/x64-windows/lib/lib)
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        link_directories(target/x64-linux/debug/lib)
    else()
        link_directories(target/x64-linux/lib)
    endif()
endif()

# 设置可执行文件目录和安装目录
if(WIN32)
    message(STATUS "ignore set (bin and install)'s dir!!!")
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/debug/bin)
    else()
        set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
    endif()
    set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)
endif()

# 设置依赖库
if(WIN32)
    set(OpensslLib libssl.lib libcrypto.lib)
    set(ThriftLib libthrift.lib libthriftnb.lib zlibd.lib event_cored.lib event_extrad.lib eventd.lib)
    set(CurlLib libcurl-d.lib)
    set(ALL_LIBS ${OpensslLib} ${ThriftLib} ${CurlLib})
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ThriftLib thriftd thriftnbd thriftzd event_core event_extra event)
        set(CurlLib curl-d)
    else()
        set(ThriftLib thrift thriftnb thriftz event_core event_extra event)
        set(CurlLib curl)
    endif()
    set(OpensslLib ssl crypto)
    set(ThreadLib pthread)
    set(ALL_LIBS ${OpensslLib} ${ThriftLib} ${CurlLib} ${ThreadLib})
endif()

# 单元测试
if(BUILD_TESTING)
    message(STATUS "Building with tests")
    add_subdirectory(test)
else()
    message(STATUS "Building without tests")
endif()

# 递归选出所有cpp文件
FILE(GLOB_RECURSE SRC_LIST ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 移除thrif编译后的simple server示例
LIST(REMOVE_ITEM SRC_LIST ${CMAKE_SOURCE_DIR}/src/gen-cpp/TestService_server.skeleton.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})

if(UNIX)
    # 查找依赖的动态库：一般*.so为软连接，所以需要install所有so库
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        FILE(GLOB DEP_LIBS "${CMAKE_SOURCE_DIR}/target/x64-linux/debug/lib/*.so.*")
    else()
        FILE(GLOB DEP_LIBS "${CMAKE_SOURCE_DIR}/target/x64-linux/lib/*.so.*")
    endif()
    # 安装依赖库
    install(FILES ${DEP_LIBS} DESTINATION lib)
    # 安装可执行程序
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

# 打包：cmake编译完成之后执行以下命令
# 1. cpack -C CPackConfig.cmake：生成二进制安装包
# 2. cpack -C CPackSourceConfig.cmake：生成源码安装包
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "1")
include (CPack)
```



## 3. 其他

vs版本映射关系表：

|        -G 参数        | 内部版本 | 其他 |
| :-------------------: | :------: | :--: |
| Visual Studio 17 2022 |   v143   |      |
| Visual Studio 16 2019 |   v142   |      |
| Visual Studio 15 2017 |   v141   |      |
| Visual Studio 14 2015 |   v140   |      |



```powershell
# cmake编译指定vs版本
cmake -G "Visual Studio 16 2019" ../ -B build -DCMAKE_BUILD_TYPE=Debug

# 构建好生成器对应的工程后，直接使用cmake调用生成器执行编译
# 采用这种方式可以避免不同生成器之间的差异，从而在cmake层面实现构建方式的统一
cmake --build  [生成器工程目录，这里就是build]

# 指定win32 或者x64平台
cmake .. -G "Visual Studio 14 2015" -A x64|win32
```

