# 杂货铺

> 个人学习资料与代码样例收集

## 目录

### 编程语言与框架 (⭐ 核心学习)
| 目录 | 说明 |
|------|------|
| `py/` | Python 实用脚本（ssh、sftp、mysql、excel转换） |
| `cpp-template/` | C++ 模板技巧（SFINAE、EBCO、CRTP） |
| `qt/` | Qt 开发示例与工具类 |
| `lua/` | Lua 辅助函数封装 |

### 系统与网络
| 目录 | 说明 |
|------|------|
| `net-engine/` | 基于 redis2.8 的网络引擎（CMake 构建，Linux only） |
| `base/` ⭐ | 基础知识笔记（Linux、Docker、TCP/IP、CMake 等） |
| `base/tcp-ip学习整理/` | TCP/IP 协议学习笔记 |

### 算法与数据结构
| 目录 | 说明 |
|------|------|
| `algorithm/` | 排序算法实现（插入、希尔、归并、快速、堆排序、topK） |

### 设计模式
| 目录 | 说明 |
|------|------|
| `gof-sample/` ⭐ | 设计模式示例（C#、C++） |

### 游戏开发
| 目录 | 说明 |
|------|------|
| `behavior-tree/` | C# 行为树实现（Unity3D 用） |

### 工具与组件
| 目录 | 说明 |
|------|------|
| `timer/` | 定时器实现（时间轮、最小堆） |
| `common/` | 跨平台基础封装（datetime、thread） |
| `helper/` | C++ 辅助类（glog日志、dump崩溃检测） |

### 学习笔记
| 目录 | 说明 |
|------|------|
| `reading-note/` | 读书笔记（DDIA、Linux内核源码、Web安全） |
| `dl/` | 深度学习笔记 |
| `AI/` | 人工智能技术概述 |

### 其他
| 目录 | 说明 |
|------|------|
| `opencv/` | OpenCV 学习与实践 |
| `opengl/` | OpenGL 基础笔记 |
| `robot/` | 机器人相关资料（ROS2、RViz） |
| `rpc-note/` | RPC 技术调研 |

---

## 构建说明

- **net-engine**: `cd net-engine/build && cmake .. && make`（仅 Linux）
- **qt/qt_study**: CMake + Qt 项目
- 其他 C++ 项目：使用各目录下的 `.sln` 文件（Visual Studio）

详细说明请参考各子目录的 README 文件。