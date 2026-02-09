# 返回值优化（RVO&NRVO）

返回非基础类型值（对象）时，对



## 1. RVO&NRVO

- Return Value Optimization（返回值优化），通过把返回值构造在相应位置，从而消除返回值的拷贝和移动。
- Named Return Value Optimization（具名返回值优化），通过把返回值构造在相应位置，从而消除返回值的拷贝和移动。

```c++
struct Data {
    int a;
    std::string v;
}

Data fn_rvo() {
	return Data();
}

Data fn_nrvo() {
    Data d;
    d.a = 1;
    d.v = "abc";
	return d;
}

int main() {
    Data d = fn(); // RVO: 生效
    
    Data d1;
    d1 = fn(); // RVO: 机制失效
}
```

编译器一般将上述代码转换为：

```c++
void fn(Data& d) {
}

void fn_nrvo(Data& d) {
    d.a = 1;
    d.v = "abc";
}
```

从而避免构造多调用一次构造/拷贝构造、析构函数。

上述优化逻辑一般需要开启优化选项才会生效。



## 2. 什么情况下会失效

有些情况下，RVO&NRVO机制会失效，以下是一些常见的失效场景

- 对返回值使用move

  ```c++
  Data fn() {
  	Data d;
      return std::move(d);
  }
  ```

  这时会强制调用移动拷贝构造函数（若存在），造成RVO失效。

- 分支预测不同

  ```c++
  Data fn() {
  	Data d1, d2;
      if (flag) return d1;
      return d2;
  }
  ```

  返回相同的对象则是可以的

  ```c++
  Data fn() {
  	Data d;
      if (flag) return d;
      return d;
  }
  ```

- 先创建变量，再调用函数，也会造成优化机制失效

  ```c++
  Data d;
  d = fn();
  ```

- 返回子对象，机制也会失效

  ```c++
  std::string fn() {
      Data d;
      d.a = 1;
      d.v = "abc";
      return d.v;
  }
  ```

  

## 3. 总结

编译器进行RVO&NRVO条件有二个：

1. return 的值类型与 函数签名的返回值类型相同；
2. return的是一个局部对象；



