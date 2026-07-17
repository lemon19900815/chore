# Dokcer基础

## 1. 安装

ubuntu下使用snap直接安装，使用apt install docker还要：Docker GPG Key，添加 Docker 软件源会麻烦很多，不建议。

```sh
sudo snap install docker
```
## 2. 添加镜像源

不能访问`dockerhub`拉取镜像时，设置[镜像源](https://github.com/dongyubin/DockerHub)

```sh
sudo mkdir -p /etc/docker
sudo tee /etc/docker/daemon.json <<EOF
{
  "registry-mirrors": [
    "https://docker.1ms.run",
    "https://dockerproxy.net",
    "https://proxy.vvvv.ee",
    "https://dockerproxy.link"
  ]
}
EOF
sudo systemctl daemon-reload
sudo systemctl restart docker
```

使用snap安装的docker需要使用下面的方式重启服务（配置也不是 `/etc/docker/daemon.json`）：

查找docker的daemon.json位置：

```sh
sudo find /var/snap/docker -name daemon.json

# 我在mutipass下的配置在这里
/var/snap/docker/3505/config/daemon.json
```

```sh
sudo mkdir -p /etc/docker
sudo tee /var/snap/docker/3505/config/daemon.json <<EOF
{
  "registry-mirrors": [
    "https://docker.1ms.run",
    "https://dockerproxy.net",
    "https://proxy.vvvv.ee",
    "https://dockerproxy.link"
  ]
}
EOF
sudo snap restart docker
```

```sh
# 查看服务列表(这里可以看到docker服务)
snap list

# 重启docker
sudo snap restart docker

# 查看服务状态
sudo snap services docker

# 查看日志
sudo snap logs docker [-f]
```

测试镜像拉取（docker compose up拉起镜像过慢）

```sh
sudo docker pull docker.1ms.run/gitlab/gitlab-ce:18.9.6-ce.0
sudo docker pull docker.1ms.run/gitlab/gitlab-runner:v18.9.0
```

## 3. 常用命令

### 3.1 容器启动

```sh
docker run/start -it 容器名/容器id
```

### 3.2 容器退出

- 容器内部退出

| 退出方式                      | 结果                       | 再次启动                   |
| ------------------------- | ------------------------ | ---------------------- |
| exit（命令）                  | 退出后，这个容器也就消失了，容器销毁ps查询不到 | docker start 容器名/容器id  |
| Ctrl+D（快捷方式）              | 退出后，这个容器也就消失了，容器销毁ps查询不到 | docker start 容器名/容器id  |
| 先按，Ctrl+P;再按，Ctrl+Q（快捷方式） | 退出容器，ps能查询到，还在后台运行       | docker attach 容器名/容器id |

- 容器外部
```sh
docker stop/kill 容器名/容器id
```

### 3.3 查看容器占用空间

命令：
```sh
docker system df
```

输出：
```
ubuntu@gitlab:~/gitlab$ sudo docker system df
TYPE            TOTAL     ACTIVE    SIZE      RECLAIMABLE
Images          2         2         6.544GB   6.544GB (100%)
Containers      2         0         6.398MB   6.398MB (100%)
Local Volumes   1         1         0B        0B
Build Cache     0         0         0B        0B
```

## 4. docker通过容器构建镜像

- 语法
  `docker commit [OPTIONS] CONTAINER [REPOSITORY[:TAG]]`
  
- OPTIONS 说明：
  - **-a:** 提交的镜像作者；
  - **-c:** 使用Dockerfile指令来创建镜像；
  - **-m:** 提交时的说明文字；
  - **-p:** 在commit时，将容器暂停。
  
- 示例（centos7.9容器内构建的编译环境，然后保存下载）

  `docker commit -a "lemon" -m "compiler env" CONTAINER(实例) centos7.9:ce`

## 5. docker镜像存储/加载本地文件

- 存储（tar文件可以再压缩为tar.gz，减少空间）

  `docker save centos7.9:ce > centos7.9-ce.tar`

- 加载

  `docker load < centos7.9-ce.tar`

## 6. docker cp : 用于容器与主机之间的数据拷贝

- 拷贝到容器中

  `docker cp file containerid:/root/test/`

- 从容器拷贝到外部

  `docker cp containerid:/root/test/file.txt ./myfolder/`

## 7. docker compose

`docker-compose.yml` 是 Docker Compose 的配置文件，用来**定义和管理多个 Docker 容器**的运行方式。

### 核心作用

一个应用通常由多个服务组成（比如 Web 服务器 + 数据库 + 缓存），`docker-compose.yml` 让你用一个文件描述所有服务，然后一条命令全部启动。

### 典型示例

```yaml
version: "3"
services:
  web:
    image: nginx
    ports:
      - "80:80"
    depends_on:
      - db

  db:
    image: postgres
    environment:
      POSTGRES_PASSWORD: secret
    volumes:
      - db_data:/var/lib/postgresql/data

volumes:
  db_data:
```

### 可以配置的内容

- **services** — 每个容器（镜像、端口、环境变量、命令等）
- **volumes** — 数据持久化挂载
- **networks** — 容器间通信的网络
- **depends_on** — 启动顺序依赖
- **environment** — 环境变量

### 常用命令

| 命令                    | 作用      |
| --------------------- | ------- |
| `docker compose up`   | 启动所有服务  |
| `docker compose down` | 停止并删除容器 |
| `docker compose logs` | 查看日志    |
| `docker compose ps`   | 查看运行状态  |

### 核心优势

- **一键启动整个项目**，不用手动启动每个容器
- **配置即代码**，可以提交到 Git 管理
- **开发环境统一**，团队成员运行结果一致

简单说：它就是你整个应用基础设施的"说明书"。

## 8. docker build

`.gitlab-ci.yml`
```yaml
image: gcc:latest

stages:
  - build

before_script:
  - apt-get update
  - apt-get install -y cmake
  - apt-get install -y ninja-build

build:
  stage: build

  script:
    - cmake -B build .
    - cmake --build build
	# 调试，打印当前工作目录
    - echo "current dir:" `pwd`
    - echo "Generated files:"
    - find build -maxdepth 2 -type f

  artifacts:
    paths:
      - build/hello

    expire_in: 7 days
```

docker运行gitlab ci时，都会执行 `apt-get update，apt-get install -y cmake`，效率很低，通过docker build构建自己的镜像。

```Dockerfile
FROM gcc:latest

RUN apt-get update && \
    apt-get install -y \
        cmake \
        ninja-build && \
    rm -rf /var/lib/apt/lists/*
```

构建：
```sh
docker build -t my-cpp-builder:1.0 .
```

然后将 `.gitlab-ci.yml` 中的镜像改为：

```yaml
image: my-cpp-builder:1.0
```

这样 Pipeline 就无需每次安装依赖，启动速度会快很多。