# Multipass使用

## 1. 设置ssh工具远程访问

在外部的powershell中生成key，用于远程访问登录虚拟机。
```powershell
ssh-keygen -t ed25519
```

生成公钥和私钥：
```
id_ed25519.pub # 公钥
id_ed25519     # 私钥
```

拷贝公钥内容复制到虚拟机下的ssh的授权文件中：
```sh
type $env:USERPROFILE\.ssh\id_ed25519.pub | ssh ubuntu@172.29.151.73 "mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys"
```

在windows环境可以通过ssh远程：
```powershell
ssh ubuntu@172.29.151.73
```

在MobaXterm中远程连接ubuntu虚拟机，需要手动指定==私钥==：

![](./img/multipass-MobaXterm.png)

## 2. 数据交互
宿主机与multipass虚拟机之间交换数据最简单的方式就是挂载目录：

- 开启挂载权限，否则之后执行挂载会失败：
```powershell
multipass set local.privileged-mounts=true
```

- 开启挂载：
```powershell
multipass mount D:\work cute-chickadee:/mnt/work
```

- 查询挂载信息：==Mounts==
```powershell
C:\Users\buerjia>multipass info cute-chickadee
Name:           cute-chickadee
State:          Running
Snapshots:      0
IPv4:           172.17.190.157
Release:        Ubuntu 24.04.4 LTS
Image hash:     53fdde898fee (Ubuntu 24.04 LTS)
CPU(s):         1
Load:           0.73 0.30 0.11
Disk usage:     2.6GiB out of 4.8GiB
Memory usage:   394.7MiB out of 896.1MiB
Mounts:         D:\workspace => /mnt/workspace
                    UID map: -2:default
                    GID map: -2:default
```

## 3. multipass虚拟机设置固定ip
multipass每次虚拟机启动时，ip会发生变化，由DHCP动态分配，造成像`MobaXterm`这类工具不能访问。

- 查看之前配置信息
```sh
ubuntu@cute-chickadee:/mnt/workspace$ sudo cat /etc/netplan/50-cloud-init.yaml 
```
```yaml
network:
  version: 2
  ethernets:
    default:
      match:
        macaddress: "52:54:00:3a:a8:6a"
      dhcp-identifier: "mac"
      dhcp4: true
```

- 使用 `ip addr` 查看当前ip，然后修改 `50-cloud-init.yaml` 配置：
```yaml
network:
  version: 2
  ethernets:
    default:
      match:
        macaddress: "52:54:00:3a:a8:6a"
      dhcp-identifier: "mac"
      dhcp4: false                    # 关闭 DHCP
      addresses:
        - 172.17.190.157/20           # 你想要的固定 IP（注意子网段）
      nameservers:
        addresses: [8.8.8.8, 1.1.1.1]
```

- 操作步骤
```sh
# 1. 备份原配置
sudo cp /etc/netplan/50-cloud-init.yaml /etc/netplan/50-cloud-init.yaml.bak

# 2. 编辑配置（替换为上面方法一的内容）
sudo nano /etc/netplan/50-cloud-init.yaml

# 3. 检查语法
sudo netplan generate

# 4. 应用（注意：SSH 连接可能中断，需要用 multipass shell 进入）
sudo netplan apply
```

**注意：** 操作完成之后，==重启multipass服务==。