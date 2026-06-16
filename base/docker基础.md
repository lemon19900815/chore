# Dokcer基础

## 1. 添加镜像源

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



## 2. 容器启动

`docker run/start -it 容器名/容器id`

## 3. 容器退出

- 容器内部退出

  | 退出方式                              | 结果                                           | 再次启动                    |
  | ------------------------------------- | ---------------------------------------------- | --------------------------- |
  | exit（命令）                          | 退出后，这个容器也就消失了，容器销毁ps查询不到 | docker start 容器名/容器id  |
  | Ctrl+D（快捷方式）                    | 退出后，这个容器也就消失了，容器销毁ps查询不到 | docker start 容器名/容器id  |
  | 先按，Ctrl+P;再按，Ctrl+Q（快捷方式） | 退出容器，ps能查询到，还在后台运行             | docker attach 容器名/容器id |

- 容器外部

  docker stop 容器名/容器id

  docker start 容器名/容器id

## 4. docker通过容器构建镜像

- 语法
  `docker commit [OPTIONS] CONTAINER [REPOSITORY[:TAG]]`
  
- OPTIONS 说明：
  - **-a : ** 提交的镜像作者；
  - **-c :** 使用Dockerfile指令来创建镜像；
  - **-m :** 提交时的说明文字；
  - **-p : ** 在commit时，将容器暂停。
  
- 示例（centos7.9容器内构建的编译环境，然后保存下载）

  `docker commit -a "lemon" -m "compiler env" CONTAINER(实例) centos7.9:ce`

## 5. docker镜像存储/加载本地文件

- 存储（tar文件可以再压缩为tar.gz，减少空间）

  `docker save centos7.9:ce > centos7.9-ce.tar`

- 加载

  `docker load < centos7.9-ce.tar`

## 6. **docker cp :**用于容器与主机之间的数据拷贝。

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

| 命令                  | 作用           |
| --------------------- | -------------- |
| `docker compose up`   | 启动所有服务   |
| `docker compose down` | 停止并删除容器 |
| `docker compose logs` | 查看日志       |
| `docker compose ps`   | 查看运行状态   |

### 核心优势

- **一键启动整个项目**，不用手动启动每个容器
- **配置即代码**，可以提交到 Git 管理
- **开发环境统一**，团队成员运行结果一致

简单说：它就是你整个应用基础设施的"说明书"。