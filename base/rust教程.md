# Rust教程

[菜鸟教程](https://www.runoob.com/rust/rust-tutorial.html)

[rust-npm仓库](https://crates.io/)



## 1. 安装

- 下载与系统对应的版本：https://rust-lang.org/zh-CN/tools/install/；
  - rust需要依赖c编译器，windows下默认使用`msvc`，可自定义选择；
  - rust在windows下安装之后，默认位于`C:\Users(用户)\lemon\.cargo`下；

- 安装`vscode`，安装插件：Chinese、rust-analyzer 和 Native Debug；
- 安装之后运行`rustc -V`和`cargo -V`查看对应的版本；



## 2. 初探

- 新建一个工作目录，如：`rust_ws`；

- `vscode`中新建一个终端，并运行：`cargo new greeting `，greeting为工程目录；

  在终端中继续运行以下三个命令：

  ```sh
  cd ./greeting 
  cargo build 
  cargo run 
  ```

  然后终端会输出：Hello，World！

  至此，你成功的构建了一个 Rust 命令行程序！



## 3. Cargo

在 Rust 开发中，几乎所有的项目都是使用 Cargo 来进行管理和构建的，因为它提供了便捷的工作流程和强大的功能，使得 Rust 开发变得更加高效和可靠。

### 3.1 Cargo 是什么

Cargo 是 Rust 的官方构建系统和包管理器。

主要有两个作用：

- **项目管理**：Cargo 用于创建、构建和管理 Rust 项目。通过 Cargo，你可以轻松地创建新项目，管理项目的依赖关系，并执行项目的构建、运行和测试等操作。
- **包管理器**：Cargo 还充当了 Rust 的包管理器。它允许开发者在项目中引入和管理依赖项（如第三方库），并确保这些依赖项的版本管理和兼容性。



### 3.2 Cargo 功能

Cargo 除了创建工程以外还具备构建（build）工程、运行（run）工程等一系列功能，构建和运行分别对应以下命令：

- `cargo new <project-name>`：创建一个新的 Rust 项目。
- `cargo build`：编译当前项目。
- `cargo run`：编译并运行当前项目。
- `cargo check`：检查当前项目的语法和类型错误。
- `cargo test`：运行当前项目的单元测试。
- `cargo update`：更新 `Cargo.toml` 中指定的依赖项到最新版本。
- `cargo --help`：查看 Cargo 的帮助信息。
- `cargo publish`：将 Rust 项目发布到 crates.io。
- `cargo clean`：清理构建过程中生成的临时文件和目录。



### 3.3 在 VSCode 中配置 Rust 工程

Cargo 是一个不错的构建工具，如果使 VSCode 与它相配合那么 VSCode 将会是一个十分便捷的开发环境。

在上一章中我们建立了 greeting 工程，现在我们用 VSCode 打开 greeting 文件夹（**注意不是 runoob-greeting**）。

打开 greeting 之后，在里面新建一个新的文件夹 **.vscode** （注意 vscode 前面的点，如果有这个文件夹就不需要新建了）。在新建的 .vscode 文件夹里新建两个文件 tasks.json 和 launch.json，文件内容如下：

```json
{ 
    "version": "2.0.0", 
    "tasks": [ 
        { 
            "label": "build", 
            "type": "shell", 
            "command":"cargo", 
            "args": ["build"] 
        } 
    ] 
}
```

launch.json 文件（适用在 Windows 系统上）

```json
{ 
    "version": "0.2.0", 
    "configurations": [ 
        { 
            "name": "(Windows) 启动", 
            "preLaunchTask": "build", 
            "type": "cppvsdbg", 
            "request": "launch", 
            "program": "${workspaceFolder}/target/debug/${workspaceFolderBasename}.exe", 
            "args": [], 
            "stopAtEntry": false, 
            "cwd": "${workspaceFolder}", 
            "environment": [], 
            "console": "integratedTerminal"
        }
    ] 
}
```

然后点击 VSCode 左栏的 "运行"。

如果你使用的是 MSVC 选择 "(Windows) 启动"。



## 4. Rust基础语法

### 4.1 示例程序窥探基础语法

通过以下示例代码，可以窥见Rust的一些基础语法：

```rust

fn add(x: i32, y: i32)->i32 {
    return x+y;
    // 函数的最后一行表达式，可以不使用return
}

#[derive(Debug)] // 自动为结构体实现 Debug trait
struct User {
    username: String,
    email: String,
    sign_in_count: u64,
    active: bool,
}

enum Coin {
    Penny,
    Nickel,
    Dime,
    Quarter,
}

fn value_in_cents(coin: Coin) -> u8 {
    match coin {
        Coin::Penny => 1,
        Coin::Nickel => 5,
        Coin::Dime => 10,
        Coin::Quarter => 25,
    }
}

fn main() {
    let x = 123;
    let y  = 456;
    let z = add(x, y);
    println!("{}+{}={}", x, y, z);

    if z > 5 && z < 1000 {
        println!("{0}>5 && {0}<1000", z)
    } else {
        println!("{}<5", z)
    }

    // loop无限循环，使用break退出
    let mut counter = 0;
    loop {
        counter += 1;
        println!("loop: {}!", counter);
        
        if counter == 10 {
            break;
        }
    }

    // while循环
    let mut number = 3;
    while number != 0 {
        println!("while: {}!", number);
        number -= 1;
    }

    // for循环
    for number in 5..8 {
        println!("for: {}!", number);
    }
    
    let a = "test01";
    let d = a;
    println!("a: {}", a);

    // complie error
    // let s = String::from("value");
    // let s2 = s;
    // println!("b = {}", s)

    let u = User{
        username: String::from("abc"),
        email: String::from("chore@github.com"),
        sign_in_count: 1,
        active: false,
    };

    println!("{:#?}", u);
    println!("Coin::Penny = {}", value_in_cents(Coin::Penny));
}
```



### 4.2 所有权 (Ownership)

所有权对大多数开发者而言是一个新颖的概念，它是 Rust 语言为高效使用内存而设计的语法机制。所有权概念是为了让 Rust 在编译阶段更有效地分析内存资源的有用性以实现内存管理而诞生的概念。

Rust 中的所有权是独特的内存管理机制，核心概念包括所有权 (ownership)、借用 (borrowing) 和引用 (reference)。

**所有权规则:**

所有权有以下三条规则：

- Rust 中的每个值都有一个所有者。
- 每个值在任意时刻只能有一个所有者。
- 当所有者超出作用域时，值会被删除。

这三条规则是所有权概念的基础。

```rust
let s1 = String::from("hello");
let s2 = s1; // s1 的所有权被转移给了 s2
// println!("{}", s1); // 此处编译会报错，因为 s1 已不再拥有该值
```

**引用与租借**

引用（Reference）是 C++ 开发者较为熟悉的概念。

如果你熟悉指针的概念，你可以把它看作一种指针。

实质上"引用"是变量的间接访问方式。

```rust
fn main() {
    let s1 = String::from("hello");
    let s2 = &s1;
    println!("s1 is {}, s2 is {}", s1, s2);
}
```

运行结果：

```
s1 is hello, s2 is hello
```

**&** 运算符可以取变量的"引用"。

当一个变量的值被引用时，变量本身不会被认定无效。因为"引用"并没有在栈中复制变量的值：

```mermaid
graph LR
    subgraph S["栈 Stack"]
        subgraph A["s1: String"]
        	B["ptr"]
        end
        C["s2: &String"] --> |引用| B
    end
    
    subgraph H["堆 Heap"]
        B --> D["Hello"]
    end
    
    style A fill:#ffe6e6,stroke:#ff0000,stroke-width:2px
    style C fill:#fff0e6,stroke:#ff9900,stroke-width:2px
    style B fill:#e6f2ff,stroke:#3399ff,stroke-width:1px
    style D fill:#e6ffe6,stroke:#00cc00,stroke-width:2px
```



函数参数传递的道理一样：

```rust
fn main() {
    let s = String::from("hello");
    let len = calculate_length(&s);  // 借用
    println!("The length of '{}' is {}.", s, len);
}

fn calculate_length(s: &String) -> usize {
    s.len()
}
```

引用不会获得值的所有权。

引用只能租借（Borrow）值的所有权。

引用本身也是一个类型并具有一个值，这个值记录的是别的值所在的位置，但引用不具有所指值的所有权：

```rust
fn main() {
    let s1 = String::from("hello");
    let s2 = &s1;
    let s3 = s1; // 编译错误
    s2 = &s3;    // 重新租借
    println!("{}", s2);
}
```

这段程序不正确：因为 s2 租借的 s1 已经将所有权移动到 s3，所以 s2 将无法继续租借使用 s1 的所有权。如果需要使用 s2 使用该值，必须重新租借。

既然引用不具有所有权，即使它租借了所有权，它也只享有使用权（这跟租房子是一个道理）。

如果尝试利用租借来的权利来修改数据会被阻止：

```rust
fn main() {
    let s1 = String::from("run");
    let s2 = &s1;
    println!("{}", s2);
    s2.push_str("oob"); // 错误，禁止修改租借的值
    println!("{}", s2);
}
```

这段程序中 s2 尝试修改 s1 的值被阻止，租借的所有权不能修改所有者的值。

当然，也存在一种可变的租借方式，就像你租一个房子，如果物业规定房主可以修改房子结构，房主在租借时也在合同中声明赋予你这种权利，你是可以重新装修房子的：

```rust
fn main() {
    let mut s1 = String::from("run");
    // s1 是可变的

    let s2 = &mut s1;
    // s2 是可变的引用

    s2.push_str("oob");
    println!("{}", s2);
}
```

这段程序就没有问题了。我们用 **&mut**  修饰**可变的引用类型**。

可变引用与不可变引用相比除了权限不同以外，可变引用不允许多重引用，但不可变引用可以。

```rust
let mut s = String::from("hello");

let r1 = &mut s;
let r2 = &mut s; // 错误

println!("{}, {}", r1, r2);
```

这段程序不正确，因为多重可变引用了 s。

Rust 对可变引用的这种设计主要出于对并发状态下发生数据访问碰撞的考虑，在编译阶段就避免了这种事情的发生。

由于发生数据访问碰撞的必要条件之一是数据被至少一个使用者写且同时被至少一个其他使用者读或写，所以在一个值被可变引用时不允许再次被任何引用。

**克隆**

Rust会尽可能地降低程序的运行成本，所以默认情况下，长度较大的数据存放在堆中，且采用移动的方式进行数据交互。但如果需要将数据单纯的复制一份以供他用，可以使用数据的第二种交互方式——克隆。

```rust
fn main() {
    let s1 = String::from("hello");
    let s2 = s1.clone();
    println!("s1 = {}, s2 = {}", s1, s2);
}
```

这里是真的将堆中的 "hello" 复制了一份，所以 s1 和 s2 都分别绑定了一个值，释放的时候也会被当作两个资源。

当然，克隆仅在需要复制的情况下使用，毕竟复制数据会花费更多的时间。



### 4.3 闭包

闭包相当于 Rust 中的 Lambda 表达式，格式如下：

```
|参数1, 参数2, ...| -> 返回值类型 {
    // 函数体
}
```

如果外部参数需要在lambda中被使用，可以使用**move**关键字

```
move |参数1, 参数2, ...| -> 返回值类型 {
    // 函数体(使用外部参数)
}
```

闭包在 Rust 中是一种特殊的类型，它们分别表示不同的闭包特性：

- `Fn`：闭包不可变地借用其环境中的变量。
- `FnMut`：闭包可变地借用其环境中的变量。
- `FnOnce`：闭包获取其环境中的变量的所有权，只能被调用一次。

```rust
// 定义一个函数，接受一个闭包作为参数，将闭包应用到给定的数字上
fn apply_operation<F>(num: i32, operation: F) -> i32
	where F: Fn(i32) -> i32,
{
    operation(num)
}

// 主函数
fn main() {
    // 定义一个数字
    let num = 5;

    // 定义一个闭包，用于对数字进行平方运算
    let square = |x| x * x;

    // 调用函数，并传入闭包作为参数，对数字进行平方运算
    let result = apply_operation(num, square);

    // 输出结果
    println!("Square of {} is {}", num, result);
}
```

闭包可以省略类型声明使用 Rust 自动类型判断机制

```rust
fn main() {
    let inc = |num| {
        num + 1
    };
    println!("inc(5) = {}", inc(5));
}
```



### 4.4 Rust Slice（切片）类型（str vs String）

**字符串切片**

最简单、最常用的数据切片类型是字符串切片（String Slice）

```rust
fn main() {
    let s = String::from("broadcast");

    let part1 = &s[0..5];
    let part2 = &s[5..9];

    println!("{}={}+{}", s, part1, part2);
}
```

![](./img/rust-slice1.png)

上图解释了字符串切片的原理（注：Rust 中的字符串类型实质上记录了字符在内存中的起始位置和其长度，我们暂时了解到这一点）。

使用 **..** 表示范围的语法在循环章节中出现过。**x..y** 表示 **[x, y)** 的数学含义。**..** 两边可以没有运算数：

```
..y 等价于 0..y
x.. 等价于位置 x 到数据结束
.. 等价于位置 0 到结束
```

**注意：**到目前为止，尽量不要在字符串中使用非英文字符，因为编码的问题。具体原因会在"字符串"章节叙述。

被切片引用的字符串禁止更改其值：

```rust
fn main() {
    let mut s = String::from("runoob");
    let slice = &s[0..3];
    s.push_str("yes!"); // 错误
    println!("slice = {}", slice);
}
```

这段程序不正确。

s 被部分引用，禁止更改其值。



**在 Rust 中有两种常用的字符串类型：str 和 String**

str 是 Rust 核心语言类型，就是字符串切片（String Slice），常常以引用的形式出现（&str）。

凡是用双引号包括的字符串常量整体的类型性质都是 **&str**：

```rust
let s = "hello";
```

这里的 s 就是一个 &str 类型的变量。

String 类型是 Rust 标准公共库提供的一种数据类型，它的功能更完善——它支持字符串的追加、清空等实用的操作。String 和 str 除了同样拥有一个字符开始位置属性和一个字符串长度属性以外还有一个容量（capacity）属性。

String 和 str 都支持切片，切片的结果是 &str 类型的数据。

注意：切片结果必须是引用类型，但开发者必须自己明示这一点:

```rust
let slice = &s[0..3];
```

有一个快速的办法可以将 String 转换成 &str：

```rust
let s1 = String::from("hello");
let s2 = &s1[..];
```

**非字符串切片**

除了字符串以外，其他一些线性数据结构也支持切片操作，例如数组：

```rust
fn main() {
    let arr = [1, 3, 5, 7, 9];
    let part = &arr[0..3];
    for i in part.iter() {
        println!("{}", i);
    }
}
```



### 4.5 组织管理

任何一门编程语言如果不能组织代码都是难以深入的，几乎没有一个软件产品是由一个源文件编译而成的。

本教程到目前为止所有的程序都是在一个文件中编写的，主要是为了方便学习 Rust 语言的语法和概念。

对于一个工程来讲，组织代码是十分重要的。

Rust 中有三个重要的组织概念：箱、包、模块。

**箱（Crate）**

"箱"是二进制程序文件或者库文件，存在于"包"中。

"箱"是树状结构的，它的树根是编译器开始运行时编译的源文件所编译的程序。

注意："二进制程序文件"不一定是"二进制可执行文件"，只能确定是是包含目标机器语言的文件，文件格式随编译环境的不同而不同。

**包（Package）**

当我们使用 Cargo 执行 new 命令创建 Rust 工程时，工程目录下会建立一个 Cargo.toml 文件。工程的实质就是一个包，包必须由一个 Cargo.toml 文件来管理，该文件描述了包的基本信息以及依赖项。

一个包最多包含一个库"箱"，可以包含任意数量的二进制"箱"，但是至少包含一个"箱"（不管是库还是二进制"箱"）。

当使用 cargo new 命令创建完包之后，src 目录下会生成一个 main.rs 源文件，Cargo 默认这个文件为二进制箱的根，编译之后的二进制箱将与包名相同。

**模块（Module）**

对于一个软件工程来说，我们往往按照所使用的编程语言的组织规范来进行组织，组织模块的主要结构往往是树。Java 组织功能模块的主要单位是类，而 JavaScript 组织模块的主要方式是 function。

这些先进的语言的组织单位可以层层包含，就像文件系统的目录结构一样。Rust 中的组织单位是模块（Module）。

```rust
mod nation {
    mod government {
        fn govern() {}
    }
    mod congress {
        fn legislate() {}
    }
    mod court {
        fn judicial() {}
    }
}
```

这是一段描述法治国家的程序：国家（nation）包括政府（government）、议会（congress）和法院（court），分别有行政、立法和司法的功能。我们可以把它转换成树状结构：

```
nation
 ├── government
 │ └── govern
 ├── congress
 │ └── legislate
 └── court
   └── judicial
```

在文件系统中，目录结构往往以斜杠在路径字符串中表示对象的位置，Rust 中的路径分隔符是 **::** 。

路径分为绝对路径和相对路径。绝对路径从 crate 关键字开始描述。相对路径从 self 或 super 关键字或一个标识符开始描述。例如：

```
crate::nation::government::govern();
```

是描述 govern 函数的绝对路径，相对路径可以表示为：

```
nation::government::govern();
```

现在你可以尝试在一个源程序里定义类似的模块结构并在主函数中使用路径。

如果你这样做，你一定会发现它不正确的地方：government 模块和其中的函数都是私有（private）的，你不被允许访问它们。

**访问权限**

Rust 中有两种简单的访问权：公共（public）和私有（private）。

默认情况下，如果不加修饰符，模块中的成员访问权将是私有的。

如果想使用公共权限，需要使用 pub 关键字。

对于私有的模块，只有在与其平级的位置或下级的位置才能访问，不能从其外部访问。

```rust
mod nation {
    pub mod government {
        pub fn govern() {}
    }

    mod congress {
        pub fn legislate() {}
    }
   
    mod court {
        fn judicial() {
            super::congress::legislate();
        }
    }
}

fn main() {
    nation::government::govern();
}
```

这段程序是能通过编译的。请注意观察 court 模块中 super 的访问方法。

如果模块中定义了结构体，结构体除了其本身是私有的以外，其字段也默认是私有的。所以如果想使用模块中的结构体以及其字段，需要 pub 声明：

**难以发现的模块**

使用过 Java 的开发者在编程时往往非常讨厌最外层的 class 块——它的名字与文件名一模一样，因为它就表示文件容器，尽管它很繁琐但我们不得不写一遍来强调"这个类是文件所包含的类"。

不过这样有一些好处：起码它让开发者明明白白的意识到了类包装的存在，而且可以明确的描述类的继承关系。

在 Rust 中，模块就像是 Java 中的类包装，但是文件一开头就可以写一个主函数，这该如何解释呢？

每一个 Rust 文件的内容都是一个"难以发现"的模块。

让我们用两个文件来揭示这一点：

```rust
// second_module.rs
pub fn message() -> String {
    String::from("This is the 2nd module.")
}
```



```rust
// main.rs
mod second_module;

fn main() {
    println!("This is the main module.");
    println!("{}", second_module::message());
}
```

**use 关键字**

use 关键字能够将模块标识符引入当前作用域：

```rust
mod nation {
    pub mod government {
        pub fn govern() {}
    }
}

use crate::nation::government::govern;

fn main() {
    govern();
}
```

这段程序能够通过编译。

因为 use 关键字把 govern 标识符导入到了当前的模块下，可以直接使用。

这样就解决了局部模块路径过长的问题。

当然，有些情况下存在两个相同的名称，且同样需要导入，我们可以使用 as 关键字为标识符添加别名：

```rust
mod nation {
    pub mod government {
        pub fn govern() {}
    }
    pub fn govern() {}
}
   
use crate::nation::government::govern;
use crate::nation::govern as nation_govern;

fn main() {
    nation_govern();
    govern();
}
```

**引用标准库**

Rust 官方标准库字典：https://doc.rust-lang.org/stable/std/all.html

在学习了本章的概念之后，我们可以轻松的导入系统库来方便的开发程序了：

```rust
use std::f64::consts::PI;

fn main() {
    println!("{}", (PI / 2.0).sin());
}
```

所有的系统库模块都是被默认导入的，所以在使用的时候只需要使用 use 关键字简化路径就可以方便的使用了。



### 4.6 泛型与特性

泛型是一个编程语言不可或缺的机制。

C++ 语言中用"模板"来实现泛型，而 C 语言中没有泛型的机制，这也导致 C 语言难以构建类型复杂的工程。

泛型机制是编程语言用于表达类型抽象的机制，一般用于功能确定、数据类型待定的类，如链表、映射表等。

```rust
fn max<T: PartialOrd>(array: &[T]) -> &T {
    let mut max_index = 0;
    let mut i = 1;
    while i < array.len() {
        if array[i] > array[max_index] {
            max_index = i;
        }
        i += 1;
    }
    return &array[max_index];
}
```

**结构体和枚举中的泛型**

在之前我们学习的 Option 和 Result 枚举类就是泛型的。

Rust 中的结构体和枚举类都可以实现泛型机制。

```rust
struct Point<T> {
    x: T,
    y: T
}
```

这是一个点坐标结构体，T 表示描述点坐标的数字类型。我们可以这样使用：

```
let p1 = Point {x: 1, y: 2};
let p2 = Point {x: 1.0, y: 2.0};
```

使用时并没有声明类型，这里使用的是自动类型机制，但不允许出现类型不匹配的情况如下：

```
let p = Point {x: 1, y: 2.0};
```

x 与 1 绑定时就已经将 T 设定为 i32，所以不允许再出现 f64 的类型。如果我们想让 x 与 y 用不同的数据类型表示，可以使用两个泛型标识符：

```
struct Point<T1, T2> {
    x: T1,
    y: T2
}
```

在枚举类中表示泛型的方法诸如 Option 和 Result：

```
enum Option<T> {
    Some(T),
    None,
}

enum Result<T, E> {
    Ok(T),
    Err(E),
}
```

结构体与枚举类都可以定义方法，那么方法也应该实现泛型的机制，否则泛型的类将无法被有效的方法操作。

```rust
struct Point<T> {
    x: T,
    y: T,
}

impl<T> Point<T> {
    fn x(&self) -> &T {
        &self.x
    }
}

fn main() {
    let p = Point { x: 1, y: 2 };
    println!("p.x = {}", p.x());
}
```

**特性（trait）**

特性（trait）概念接近于 Java 中的接口（Interface），但两者不完全相同。特性与接口相同的地方在于它们都是一种行为规范，可以用于标识哪些类有哪些方法。

特性在 Rust 中用 trait 表示：（特性和C++的接口类似，可以提供默认的实现）

```
trait Descriptive {
    fn describe(&self) -> String;
}
```

Descriptive 规定了实现者必需有 **describe(&self) -> String** 方法。

我们用它实现一个结构体：

```rust
struct Person {
    name: String,
    age: u8
}

impl Descriptive for Person {
    fn describe(&self) -> String {
        format!("{} {}", self.name, self.age)
    }
}
```

格式是：

```
impl <特性名> for <所实现的类型名>
```

Rust 同一个类可以实现多个特性，每个 impl 块只能实现一个。

**特性做参数**

很多情况下我们需要传递一个函数做参数，例如回调函数、设置按钮事件等。在 Java 中函数必须以接口实现的类实例来传递，在 Rust 中可以通过传递特性参数来实现：

```
fn output(object: impl Descriptive) {
    println!("{}", object.describe());
}
```

任何实现了 Descriptive 特性的对象都可以作为这个函数的参数，这个函数没必要了解传入对象有没有其他属性或方法，只需要了解它一定有 Descriptive 特性规范的方法就可以了。当然，此函数内也无法使用其他的属性与方法。

特性参数还可以用这种等效语法实现：

```
fn output<T: Descriptive>(object: T) {
    println!("{}", object.describe());
}
```

这是一种风格类似泛型的语法糖，这种语法糖在有多个参数类型均是特性的情况下十分实用：

```
fn output_two<T: Descriptive>(arg1: T, arg2: T) {
    println!("{}", arg1.describe());
    println!("{}", arg2.describe());
}
```

特性作类型表示时如果涉及多个特性，可以用 **+** 符号表示，例如：

```
fn notify(item: impl Summary + Display)
fn notify<T: Summary + Display>(item: T)
```

**注意：**仅用于表示类型的时候，并不意味着可以在 impl 块中使用。

复杂的实现关系可以使用 where 关键字简化，例如：

```
fn some_function<T: Display + Clone, U: Clone + Debug>(t: T, u: U)
```

可以简化成：

```
fn some_function<T, U>(t: T, u: U) -> i32
    where T: Display + Clone,
          U: Clone + Debug
```

在了解这个语法之后，泛型章节中的"取最大值"案例就可以真正实现了：

```rust
trait Comparable {
    fn compare(&self, object: &Self) -> i8;
}

fn max<T: Comparable>(array: &[T]) -> &T {
    let mut max_index = 0;
    let mut i = 1;
    while i < array.len() {
        if array[i].compare(&array[max_index]) > 0 {
            max_index = i;
        }
        i += 1;
    }
    return &array[max_index];
}

impl Comparable for f64 {
    fn compare(&self, object: &f64) -> i8 {
        if &self > &object { 1 }
        else if &self == &object { 0 }
        else { -1 }
    }
}

fn main() {
    let arr = [1.0, 3.0, 5.0, 4.0, 2.0];
    println!("maximum of arr is {}", max(&arr));
}
```



### 4.7 面向对象

面向对象的编程语言通常实现了数据的封装与继承并能基于数据调用方法。

Rust 不是面向对象的编程语言，但这些功能都得以实现。

**封装**

封装就是对外显示的策略，在 Rust 中可以通过模块的机制来实现最外层的封装，并且每一个 Rust 文件都可以看作一个模块，模块内的元素可以通过 pub 关键字对外明示。这一点在"组织管理"章节详细叙述过。

"类"往往是面向对象的编程语言中常用到的概念。"类"封装的是数据，是对同一类数据实体以及其处理方法的抽象。在 Rust 中，我们可以使用结构体或枚举类来实现类的功能：

```rust
//second.rs
pub struct ClassName {
    field: i32,
}

impl ClassName {
    pub fn new(value: i32) -> ClassName {
        ClassName {
            field: value
        }
    }

    pub fn public_method(&self) {
        println!("from public method");
        self.private_method();
    }

    fn private_method(&self) {
        println!("from private method");
    }
}
//main.rs
mod second;
use second::ClassName;

fn main() {
    let object = ClassName::new(1024);
    object.public_method();
}
```

**继承**

几乎其他的面向对象的编程语言都可以实现"继承"，并用"extend"词语来描述这个动作。

继承是多态（Polymorphism）思想的实现，多态指的是编程语言可以处理多种类型数据的代码。在 Rust 中，通过特性（trait）实现多态。有关特性的细节已在"特性"章节给出。但是特性无法实现属性的继承，只能实现类似于"接口"的功能，所以想继承一个类的方法最好在"子类"中定义"父类"的实例。

总结地说，Rust 没有提供跟继承有关的语法糖，也没有官方的继承手段（完全等同于 Java 中的类的继承），但灵活的语法依然可以实现相关的功能。



### 4.8 智能指针

智能指针（Smart pointers）是一种在 Rust 中常见的数据结构，它们提供了额外的功能和安全性保证，以帮助管理内存和数据。

在 Rust 中，智能指针是一种封装了对动态分配内存的所有权和生命周期管理的数据类型。

智能指针通常封装了一个原始指针，并提供了一些额外的功能，比如引用计数、所有权转移、生命周期管理等。

在 Rust 中，标准库提供了几种常见的智能指针类型。

| 智能指针     | 使用场景 | 简介                                                         | 说明                                             |
| ------------ | :------: | ------------------------------------------------------------ | ------------------------------------------------ |
| `Box<T>`     |  单线程  | `Box<T> `是 Rust 中最简单的智能指针之一，它允许在堆上分配一块内存，并将值存储在这个内存中。 | 当需要在堆上分配内存时，使用 `Box<T>`            |
| `Rc<T>`      |  单线程  | `Rc<T>`（引用计数指针）允许多个所有者共享数据，它使用引用计数来跟踪数据的所有者数量，并在所有者数量为零时释放数据。 | 当需要多处共享所有权时，使用 `Rc<T>` 或 `Arc<T>` |
| `Arc<T>`     |  多线程  | `Arc<T>`（原子引用计数指针）与 `Rc<T>` 类似，但是可以安全地在多线程环境中共享数据，因为它使用原子操作来更新引用计数。 | 当需要内部可变性时，使用 `RefCell<T>`            |
| `RefCell<T>` |  单线程  | `RefCell<T>` 允许在运行时检查借用规则，它使用内部可变性来提供了一种安全的内部可变性模式，允许在不可变引用的情况下修改数据。 | 当需要线程安全的共享所有权时，使用 `Arc<T>`      |
| `Mutex<T>`   |  多线程  | `Mutex<T>` 是一个互斥锁，它保证了在任何时刻只有一个线程可以访问 Mutex 内部的数据。 | 当需要互斥访问数据时，使用 `Mutex<T>`            |
| `RwLock<T>`  |  多线程  | `RwLock<T> `是一种读取-写入锁，允许多个读取者同时访问数据，但在写入时是排他的。 | 当需要读取-写入访问数据时，使用 `RwLock<T>`      |
| `Weak<T>`    |  多线程  | `Weak<T>` 是 `Rc<T>` 的非拥有智能指针，它不增加引用计数，用于解决循环引用问题。 | 当需要解决循环引用问题时，使用 `Weak<T>`         |

**智能指针的生命周期管理**

智能指针可以帮助管理数据的生命周期，当智能指针被销毁时，它们会自动释放内存，从而避免了内存泄漏和野指针的问题。

此外，智能指针还允许在创建时指定特定的析构函数，以实现自定义的资源管理。

**示例**

下面是一个简单的 Rust 智能指针完整实例，该示例使用 Rc<T> 智能指针实现了一个简单的引用计数功能，并演示了多个所有者共享数据的情况。

```rust
// 引入所需的依赖库
use std::rc::Rc;

// 定义一个结构体，用于存储数据
#[derive(Debug)]
struct Data {
    value: i32,
}

// 主函数
fn main() {
    // 创建一个 Rc 智能指针，共享数据
    let data = Rc::new(Data { value: 5 });

    // 克隆 Rc 智能指针，增加数据的引用计数
    let data_clone1 = Rc::clone(&data);
    let data_clone2 = Rc::clone(&data);

    // 输出数据的值和引用计数
    println!("Data value: {}", data.value);
    println!("Reference count: {}", Rc::strong_count(&data));

    // 打印克隆后的 Rc 智能指针
    println!("Data clone 1: {:?}", data_clone1);
    println!("Data clone 2: {:?}", data_clone2);
}
```

### 4.9 并发编程

安全高效的处理并发是 Rust 诞生的目的之一，主要解决的是服务器高负载承受能力。

并发（concurrent）的概念是指程序不同的部分独立执行，这与并行（parallel）的概念容易混淆，并行强调的是"同时执行"。

并发往往会造成并行。

**线程**

Rust 中通过 std::thread::spawn 函数创建新线程：

```rust
use std::thread;
use std::time::Duration;

fn spawn_function() {
    for i in 0..5 {
        println!("spawned thread print {}", i);
        thread::sleep(Duration::from_millis(1));
    }
}

fn main() {
    thread::spawn(spawn_function);

    for i in 0..3 {
        println!("main thread print {}", i);
        thread::sleep(Duration::from_millis(1));
    }
}
```

std::thread::spawn 函数的参数是一个无参函数，但上述写法不是推荐的写法，我们可以使用闭包（closures）来传递函数作为参数：

```rust
use std::thread;
use std::time::Duration;

fn main() {
    thread::spawn(|| {
        for i in 0..5 {
            println!("spawned thread print {}", i);
            thread::sleep(Duration::from_millis(1));
        }
    });

    for i in 0..3 {
        println!("main thread print {}", i);
        thread::sleep(Duration::from_millis(1));
    }
}
```

**消息传递**

Rust 中一个实现消息传递并发的主要工具是通道（channel），通道有两部分组成，一个发送者（transmitter）和一个接收者（receiver）。

std::sync::mpsc 包含了消息传递的方法：

```rust
use std::thread;
use std::sync::mpsc;

fn main() {
    let (tx, rx) = mpsc::channel();

    thread::spawn(move || {
        let val = String::from("hi");
        tx.send(val).unwrap();
    });

    let received = rx.recv().unwrap();
    println!("Got: {}", received);
}
```

子线程获得了主线程的发送者 tx，并调用了它的 send 方法发送了一个字符串，然后主线程就通过对应的接收者 rx 接收到了。



### 4.10 异步编程 async/await

在现代编程中，异步编程变得越来越重要，因为它允许程序在等待 I/O 操作（如文件读写、网络通信等）时不被阻塞，从而提高性能和响应性。

异步编程是一种在 Rust 中处理非阻塞操作的方式，允许程序在执行长时间的 I/O 操作时不被阻塞，而是在等待的同时可以执行其他任务。

Rust 提供了多种工具和库来实现异步编程，包括 **async** 和 **await** 关键字、**futures** 和异步运行时（如 tokio、async-std 等），以及其他辅助工具。

- **Future**：Future 是 Rust 中表示异步操作的抽象。它是一个可能还没有完成的计算，将来某个时刻会返回一个值或一个错误。
- **async/await**：`async` 关键字用于定义一个异步函数，它返回一个 Future。`await` 关键字用于暂停当前 Future 的执行，直到它完成。

**完整示例**

以下实例展示了如何使用 async 和 await 关键字编写一个异步函数，以及如何在异步函数中执行异步任务并等待其完成。

```rust
use tokio;
use tokio::time;

async fn async_task() -> u32 {
    println!("async task, sim...");
    time::sleep(Duration::from_secs(1)).await;
    return 42;
}

async fn async_exec() {
    println!("async exec, waiting...");
    println!("async task: {}", async_task().await);
}

#[tokio::main]
async fn main() {
    async_exec().await;
    println!("async task completed!");
}
```

以上代码中，我们首先定义了一个异步函数 `async_task()`，该函数模拟了一个异步操作，使用 `time::sleep()` 方法来等待 1 秒钟，然后返回结果 42。接着定义了一个异步任务执行函数 `async_exec()`，在其中调用了异步函数，并使用 `await` 关键字等待异步任务的完成。最后在 `main` 函数中使用 `tokio::main` 宏来运行异步任务执行函数，并等待其完成。

运行该程序，可以看到程序输出了开始执行异步任务的提示，然后等待了 1 秒钟后输出了异步任务的结果，并最终输出了异步任务完成的提示:

```
async exec, waiting...
async task, sim...
async task: 42
async task completed!
```

`use tokio;`提示未引入模块是因为`dependencies`缺失：

```
[dependencies]
tokio = { version = "1", features = ["full"] }
```

~~添加后运行：cargo update。~~

这个例子演示了 Rust 中使用 `async` 和 `await` 关键字编写异步函数，以及如何在异步函数中执行异步任务并等待其完成。

以下实例使用 tokio 库执行异步 HTTP 请求，并输出响应结果：

```rust
// 引入所需的依赖库
use std::error::Error;
use tokio::runtime::Runtime;
use reqwest::get;

// 异步函数，用于执行 HTTP GET 请求并返回响应结果
async fn fetch_url(url: &str) -> Result<String, Box<dyn Error>> {
    // 使用 reqwest 发起异步 HTTP GET 请求
    let response = get(url).await?;
    let body = response.text().await?;
    Ok(body)
}

// 异步任务执行函数
async fn execute_async_task() -> Result<(), Box<dyn Error>> {
    // 发起异步 HTTP 请求
    let url = "https://jsonplaceholder.typicode.com/posts/1";
    let result = fetch_url(url).await?;
    // 输出响应结果
    println!("Response: {}", result);
    Ok(())
}

// 主函数
fn main() {
    // 创建异步运行时
    let rt = Runtime::new().unwrap();
    // 在异步运行时中执行异步任务
    let result = rt.block_on(execute_async_task());
    // 处理异步任务执行结果
    match result {
        Ok(_) => println!("Async task executed successfully!"),
        Err(e) => eprintln!("Error: {}", e),
    }
}
```

**异步编程说明**

**async关键字**

async 关键字用于定义异步函数，即返回 Future 或 impl Future 类型的函数。异步函数执行时会返回一个未完成的 Future 对象，它表示一个尚未完成的计算或操作。

异步函数可以包含 await 表达式，用于等待其他异步操作的完成。

```rust
async fn hello() -> String {
    "Hello, world!".to_string()
}
```

**await 关键字**

await 关键字用于等待异步操作的完成，并获取其结果。

await 表达式只能在异步函数或异步块中使用，它会暂停当前的异步函数执行，等待被等待的 Future 完成，然后继续执行后续的代码。

```rust
async fn print_hello() {
    let result = hello().await;
    println!("{}", result);
}
```

**异步函数返回值**

异步函数的返回值类型通常是 `impl Future<Output = T>`，其中 `T` 是异步操作的结果类型。由于异步函数的返回值是一个 Future，因此可以使用 `.await` 来等待异步操作的完成，并获取其结果。

```rust
async fn add(a: i32, b: i32) -> i32 {
    a + b
}
```

**异步块**

除了定义异步函数外，Rust 还提供了异步块的语法，可以在同步代码中使用异步操作。异步块由 `async { }` 构成，其中可以包含异步函数调用和 `await` 表达式。

```rust
async {
    let result1 = hello().await;
    let result2 = add(1, 2).await;
    println!("Result: {}, {}", result1, result2);
};
```

**异步任务执行**

在 Rust 中，异步任务通常需要在执行上下文中运行，可以使用 `tokio::main`、`async-std` 的 `task::block_on` 或 `futures::executor::block_on` 等函数来执行异步任务。这些函数会接受一个异步函数或异步块，并在当前线程或执行环境中执行它。

```rust
use async_std::task;

fn main() {
    task::block_on(print_hello());
}
```

**错误处理**

`await` 后面跟一个 `?` 操作符可以传播错误。如果 `await` 的 Future 完成时返回了一个错误，那么这个错误会被传播到调用者。

```rust
async fn my_async_function() -> Result<(), MyError> {
    some_async_operation().await?;
    // 如果 some_async_operation 出错，错误会被传播
}
```

**异步 trait 方法**

Rust 允许为 trait 定义异步方法。这使得你可以为不同类型的对象定义异步操作。

```rust
trait MyAsyncTrait {
    async fn async_method(&self) -> Result<(), MyError>;
}

impl MyAsyncTrait for MyType {
    async fn async_method(&self) -> Result<(), MyError> {
        // 异步逻辑
    }
}
```

**异步上下文**

在 Rust 中，异步代码通常在异步运行时（如 Tokio 或 async-std）中执行。这些运行时提供了调度和执行异步任务的机制。

```rust
#[tokio::main]
async fn main() {
    some_async_operation().await;
}
```

以上代码中，**#[tokio::main]** 属性宏将 **main** 函数包装在一个异步运行时中。

**异步宏**

Rust 提供了一些异步宏，如 **tokio::spawn**，用于在异步运行时中启动新的异步任务。

```rust
#[tokio::main]
async fn main() {
    let handle = tokio::spawn(async {
        // 异步逻辑
    });
    handle.await.unwrap();
}
```

**异步 I/O**

Rust 的标准库提供了异步 I/O 操作，如 **tokio::fs::File** 和 **async_std::fs::File**。

```rust
use tokio::fs::File;
use tokio::io::{self, AsyncReadExt};

#[tokio::main]
async fn main() -> io::Result<()> {
    let mut file = File::open("file.txt").await?;
    let mut contents = String::new();
    file.read_to_string(&mut contents).await?;
    println!("Contents: {}", contents);
    Ok(())
}
```

**异步通道**

Rust 的一些异步运行时提供了异步通道（如 tokio::sync::mpsc），允许在异步任务之间传递消息。

```rust
use tokio::sync::mpsc;
use tokio::spawn;

#[tokio::main]
async fn main() {
    let (tx, mut rx) = mpsc::channel(32);

    let child = spawn(async move {
        let response = "Hello, world!".to_string();
        tx.send(response).await.unwrap();
    });

    let response = rx.recv().await.unwrap();
    println!("Received: {}", response);

    child.await.unwrap();
}
```

**总结**

Rust 的异步编程模型 async/await 提供了一种简洁、高效的方式来处理异步操作。

它允许开发者以一种更自然和直观的方式来处理异步操作，同时保持了 Rust 的安全性和性能。

通过 async/await，Rust 为异步编程提供了一流的语言支持，使得编写高效且可读性强的异步程序变得更加容易。



## 5. Rust ORM

### 5.1 Diesel

https://diesel.rs/guides/getting-started

https://crates.io/crates/diesel

https://diesel.rs/guides/getting-started

```toml
[dependencies]
diesel = { version = "<version>", features = ["<postgres|mysql|sqlite>"] }
```

设置mysql环境变量（*如果仍然编译错误，可设置系统环境变量*）

```powershell
$env:MYSQLCLIENT_LIB_DIR="C:/Program Files/MySQL/MySQL Server 8.4/lib"
$env:MYSQLCLIENT_VERSION="8.4.7"

// ps：打印环境变量
$env:MYSQLCLIENT_VERSION

// cmd：打印环境变量
echo %MYSQLCLIENT_VERSION%
```

设置之后清理&重新编译

```powershell
cargo clean
cargo build
```

执行bin目录下的子程序（如果运行失败，请检查dll依赖）

```powershell
cargo run --bin show_posts
```



<font color="red">**使用步骤：**</font>

1. 创建数据库（根据迁移文件）

   ```powershell
    diesel setup
   ```

   这将创建我们的数据库（如果它还不存在）并设置初始迁移目录，该目录将包含建立Diesel设置的生成的迁移文件。请注意，迁移目录不会为空，因为初始设置迁移是自动生成的。

2. 现在我们要写一个小的CLI来管理博客（忽略我们只能从这个CLI访问数据库的事实）。我们首先需要一张桌子来存放我们的帖子。让我们为此创建一个迁移：

   ```powershell
   diesel migration generate create_posts
   ```

   Diesel CLI将按照所需的结构为我们创建两个空文件。您将看到类似于以下内容的输出：

   ```
   Creating migrations/20260101090000_create_posts/up.sql
   Creating migrations/20260101090000_create_posts/down.sql
   ```

   迁移允许我们随着时间的推移发展数据库模式。每次迁移都包含一个用于应用更改的`up.sql`文件和一个用于恢复更改的`down.sql`文件。应用并立即恢复迁移应保持数据库模式不变。

3. 接下来，我们将编写用于迁移的SQL：

   ```sql
   // up.sql
   CREATE TABLE posts (
     id INTEGER AUTO_INCREMENT PRIMARY KEY,
     title VARCHAR(255) NOT NULL,
     body TEXT NOT NULL,
     published BOOLEAN NOT NULL DEFAULT FALSE
   );
   ```

   ```sql
   // down.sql 
   DROP TABLE posts
   ```

4. 我们可以应用新的迁移：

   ```powershell
   diesel migration run
   ```

   每次都应该验证`up.sql`和`down.sql`是正确的。您可以通过以下方式快速确认您的`down.sql`正确回滚迁移：

   ```powershell
   diesel migration redo
   ```

   **关于迁移中的原始SQL的说明：**

   由于迁移是用原始SQL编写的，因此它们可以包含您使用的数据库系统的特定功能。例如，上面的`CREATE TABLE`语句使用PostgreSQL的`SERIAL`类型。如果你想使用SQLite，你需要使用`INTEGER`。[diesel GitHub存储库](https://github.com/diesel-rs/diesel/tree/main/examples)包含所有支持的后端的修改示例。请确保在页面顶部选择您正在使用的后端，以查看特定于该后端的示例。

   如果您更喜欢基于Rust代码生成迁移，diesel CLI工具在`--diff-schema`命令上提供了额外的`diesel migration generate`命令，允许基于当前模式定义和数据库生成迁移。要生成与所示的原始SQL迁移等效的迁移，您需要：

   - 创建一个包含以下内容的`schema.rs`文件：

     ```rust
     diesel::table! {
         posts (id) {
             id -> Int4,
             title -> Varchar,
             body -> Text,
             published -> Bool,
         }
     }
     ```

     [`table!`宏的文档](https://docs.diesel.rs/2.3.x/diesel/macro.table.html)包含用于此宏的语法。[`diesel::sql_types`模块](https://docs.diesel.rs/2.3.x/diesel/sql_types/index.html)提供了用于定义相关列的SQL端类型的文档。

   - 运行`diesel migration generate --diff-schema create_posts`

   这将生成迁移的`up.sql`和`down.sql`文件，其中预先填充了相关的SQL。这些预填充的迁移文件可以作为根据您的需要调整生成的SQL的起点。作为用户，您需要决定哪些修改与您的用例相关，因为Diesel永远不会为您生成完全完美的迁移。例如，对于本指南，您需要将`DEFAULT`子句添加到`published`字段，以匹配前面显示的迁移。

   **在生产中使用迁移的注意事项：**

   当准备应用程序以供生产使用时，您可能希望在应用程序初始化阶段运行迁移。您可能还希望将迁移脚本作为代码的一部分，以避免将它们复制到部署位置/映像等。

   [diesel_migrations](https://docs.rs/crate/diesel_migrations/2.2.0) crate提供了[`embed_migrations!`](https://docs.rs/diesel_migrations/2.2.0/diesel_migrations/macro.embed_migrations.html)宏，允许您在最终的二进制文件中嵌入迁移脚本。一旦你的代码使用了它，你可以简单地在你的`connection.run_pending_migrations(MIGRATIONS)`函数的开头包含`main`，以便在每次应用程序启动时运行迁移。

5. 





- 

### 5.2 sea-orm

- 安装sea-orm-cli：

  ```
  # In case you have not installed `sea-orm-cli`
  $ cargo install sea-orm-cli
  ```

- 初始化migrate：

  ```
  sea-orm-cli migrate init
  ```

  运行之后你就能看到初始化的migrate工程：

  ```
  # The folder structure will be as follows:
  .
  ├── Cargo.toml
  ├── migration
  │   ├── Cargo.toml
  │   ├── README.md
  │   └── src
  │       ├── lib.rs
  │       ├── m20220101_000001_create_table.rs
  │       └── main.rs
  └── src
      └── main.rs
  ```

- 定义迁移文件

  ```rust
  // m20220101_000001_create_bakery_table.rs
  
  use sea_orm_migration::prelude::*;
  
  pub struct Migration;
  
  impl MigrationName for Migration {
      fn name(&self) -> &str {
          "m20220101_000001_create_bakery_table" // Make sure this matches with the file name
      }
  }
  
  #[async_trait::async_trait]
  impl MigrationTrait for Migration {
      // Define how to apply this migration: Create the Bakery table.
      async fn up(&self, manager: &SchemaManager) -> Result<(), DbErr> {
          manager
              .create_table(
                  Table::create()
                      .table(Bakery::Table)
                      .col(
                          ColumnDef::new(Bakery::Id)
                              .integer()
                              .not_null()
                              .auto_increment()
                              .primary_key(),
                      )
                      .col(ColumnDef::new(Bakery::Name).string().not_null())
                      .col(ColumnDef::new(Bakery::ProfitMargin).double().not_null())
                      .to_owned(),
              )
              .await
      }
  
      // Define how to rollback this migration: Drop the Bakery table.
      async fn down(&self, manager: &SchemaManager) -> Result<(), DbErr> {
          manager
              .drop_table(Table::drop().table(Bakery::Table).to_owned())
              .await
      }
  }
  
  #[derive(Iden)]
  pub enum Bakery {
      Table,
      Id,
      Name,
      ProfitMargin,
  }
  ```

  ```rust
  // m20220101_000002_create_chef_table.rs
  
  use sea_orm_migration::prelude::*;
  
  use super::m20220101_000001_create_bakery_table::Bakery;
  
  pub struct Migration;
  
  impl MigrationName for Migration {
      fn name(&self) -> &str {
          "m_20220101_000002_create_chef_table" // Make sure this matches with the file name
      }
  }
  
  #[async_trait::async_trait]
  impl MigrationTrait for Migration {
      // Define how to apply this migration: Create the Chef table.
      async fn up(&self, manager: &SchemaManager) -> Result<(), DbErr> {
          manager
              .create_table(
                  Table::create()
                      .table(Chef::Table)
                      .col(
                          ColumnDef::new(Chef::Id)
                              .integer()
                              .not_null()
                              .auto_increment()
                              .primary_key(),
                      )
                      .col(ColumnDef::new(Chef::Name).string().not_null())
                      .col(ColumnDef::new(Chef::ContactDetails).json())
                      .col(ColumnDef::new(Chef::BakeryId).integer().not_null())
                      .foreign_key(
                          ForeignKey::create()
                              .name("fk-chef-bakery_id")
                              .from(Chef::Table, Chef::BakeryId)
                              .to(Bakery::Table, Bakery::Id),
                      )
                      .to_owned(),
              )
              .await
      }
  
      // Define how to rollback this migration: Drop the Chef table.
      async fn down(&self, manager: &SchemaManager) -> Result<(), DbErr> {
          manager
              .drop_table(Table::drop().table(Chef::Table).to_owned())
              .await
      }
  }
  
  // For ease of access
  #[derive(Iden)]
  pub enum Chef {
      Table,
      Id,
      Name,
      ContactDetails,
      BakeryId,
  }
  
  ```

  ```rust
  // migration/src/lib.rs
  
  pub use sea_orm_migration::prelude::*;
  
  // Add each migration file as a module
  mod m20220101_000001_create_bakery_table;
  mod m20220101_000002_create_chef_table;
  
  pub struct Migrator;
  
  #[async_trait::async_trait]
  impl MigratorTrait for Migrator {
      fn migrations() -> Vec<Box<dyn MigrationTrait>> {
          vec![
              // Define the order of migrations.
              Box::new(m20220101_000001_create_bakery_table::Migration),
              Box::new(m20220101_000002_create_chef_table::Migration),
          ]
      }
  }
  ```

  **重要提示**：请确保在`migration` crate中启用了以下功能。数据库驱动程序功能必须与正在使用的数据库匹配。

  ```toml
  # migration/Cargo.toml
  
  ...
  
  [dependencies.sea-orm-migration]
  version = "^0.12.0"
  features = [
  +   "sqlx-mysql",
  +   "runtime-async-std-native-tls",
  ]
  ```

- 执行迁移

  ```powershell
  $env:DATABASE_URL="mysql://root:root@localhost:3306/bakeries_db"
  sea-orm-cli migrate refresh
  ```

  

- 从数据库生成实体

  ```powershell
  # Generate entity files of database `bakeries_db` to `src/entities`
  $ sea-orm-cli generate entity \
      -u mysql://root:root@localhost:3306/bakeries_db \
      -o src/entities
  ```

  生成的实体文件：

  ```
  bakery-backend
  ├── Cargo.toml
  ├── migration
  │   └── ...
  └── src
      ├── entities
      │   ├── chef.rs
      │   ├── bakery.rs
      │   ├── mod.rs
      │   └── prelude.rs
      └── main.rs
  ```

  

- 

  

