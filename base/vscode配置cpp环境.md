# vscode配置cpp环境

## 1. 基础配置

配置插件：clangd、cmake tools、 c++

下载最新版本的cmake，clangd，ninja

编译后端：msvc，g++等（windows下需要安装msvc+sdk）

ninja：编译前端，组织组合调用编译后端工作（编译过程更友好等）

clangd：智能提示，跳转等

注意点：

- 禁用c++的Intelli Sense，它会和clangd相冲突；

- clangd可能出现不能导航的情况：

  ```
  CompileFlags:
    Add:
      - -Wno-microsoft-string-literal-from-predefined
  #--------------------这两行--------------------
      - /imsvc
      - C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Tools\MSVC\14.50.35717\include
  #--------------------这两行--------------------
    # Point clangd to the build directory where compile_commands.json is generated
    CompilationDatabase: ../build
  
  ---
  If:
    PathMatch:
      - .*\.pb\.(h|cc)$
      - .*[/\\]google[/\\]protobuf[/\\].*
  Diagnostics:
    Suppress: "*"
  
  ```

- 



## 2. vscode快捷键

- Ctrl+P：打开快速访问
- Ctrl+Shift+P：显示并运行命令

## 3. 编译优化
- vscode配置ninja+msvc编译大型c++工程时，通常会造成电脑卡顿，可以在`settings.json`中新增：`"cmake.buildArgs": ["--parallel", "8"]`限制并行编译的核心数量，但会降低编译速度；
```json
{

    "cmake.sourceDirectory": "${workspaceFolder}/src",
    "files.autoGuessEncoding": true,
    "cmake.buildArgs": ["--parallel", "8"]

}
```
- 