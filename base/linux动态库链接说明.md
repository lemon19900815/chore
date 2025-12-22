# Linux动态库链接说明

## 1. 编译时指定

在 Linux 中，动态库链接是指程序在编译时不将所有库函数的代码静态地链接到可执行文件中，而是在程序加载运行时才将库函数的代码动态地链接到内存中。这样可以减小程序的体积，避免代码冗余，并且可以在运行时灵活地加载不同版本的库。

在 Linux 中，动态库通常以`.so`（Shared Object）为后缀，而动态库链接分为两个阶段：

1. 编译阶段：在编译可执行文件时，通过指定动态库的名称或路径，告诉编译器程序需要链接的动态库。这通常通过 `-l` 选项指定库名称，例如 `-lmylibrary` 表示链接名为 `libmylibrary.so` 的动态库。
2. 运行时链接：当程序被加载运行时，动态链接器（dynamic linker）负责将程序依赖的动态库加载到内存中，并将程序中相应的动态库函数进行链接。动态链接器会根据程序中的动态库依赖关系找到正确的动态库，并将其加载到内存中供程序调用。

**说明**：`$ORIGIN`表示动态库或程序的当前目录。

常见的编译时动态库链接命令有以下几种形式：

- 指定动态库名称链接：`gcc -o myprogram myprogram.c -lmylibrary`
- 指定动态库路径链接：`gcc -o myprogram myprogram.c -L/path/to/library -lmylibrary`
- 指定多个动态库链接：`gcc -o myprogram myprogram.c -lmylibrary1 -lmylibrary2`

指定运行动态库链接路径命令`-Wl,-rpath`：

- `gcc -o myprogram myprogram.c -Wl,-rpath,/path/to/library `（在这个示例中，`-Wl` 选项用于将后面的参数传递给链接器，`-rpath` 选项用于指定运行时库路径，`/path/to/library` 应该替换为实际的动态库路径。）
- 使用`cmake`修改：`set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "./")`；（注：默认install之后会去掉`runpath`属性）

## 2. 编译后修改

使用`readelf`命令读取动态库的RUNPATH，并使用`chrpath`命令修改它。

```shell
# readelf读取RUNPATH信息
readelf -a your_program|your_lib | grep -i path

# 安装chrpath命令(ubuntu下，其他linux发行版本可能不同)
apt install chrpath

# 使用chrpath修改RUNPATH：修改程序的动态库搜索目录（相当于把当前目录加入到程序运行时库的搜索目录中）
chrpath -r '$ORIGIN' your_program
```

## 3. 动态库链接规则（顺序）

- `LD_LIBRARY_PATH`：使用环境变量指定路径；`export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH && ./your_program `

- `RUNPATH`：可执行文件中指定的`RUNPATH`，可通过`chrpath`修改；

- `/etc/ld.so.conf`：系统默认的动态库搜索路径；（e.g. `echo "/path/to/directory" > /etc/ld.so.conf.d/alg.conf && ldconfig`）



在程序运行时，可以通过设置环境变量 `LD_LIBRARY_PATH` 来指定动态链接器在加载动态库时搜索的路径，或者通过修改 `/etc/ld.so.conf` 文件添加动态库搜索路径。

## 4. 其他

- 可以使用[linuxdeployqt](https://github.com/probonopd/linuxdeployqt)工具打包软件或动态库