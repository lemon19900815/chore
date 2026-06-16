- 像云服务商一样快速（==几秒内==）拉起windows和ubuntu虚拟机。
- [virtual box](./virtualbox.md) 和 VMware Workstation 都不能满足我们的要求，并且占用资源多（内存和磁盘）。

下面介绍2种方式达到这一效果。
## 1. sandbox虚拟机
==注意：sandbox关机之后，数据丢失不会保留任何数据。==

### 1.1 开启 Windows Sandbox：
1. 确认开启虚拟化：在powershell中运行`Get-ComputerInfo -property HyperV*`；
2. 运行`OptionalFeatures.exe`，勾选：`Windows 沙盒`；
3. 重启电脑；
### 1.2 高阶玩法：配置文件
可以创建：sandbox.wsb
```xml
<Configuration>
  <MappedFolders>
    <MappedFolder>
      <HostFolder>D:\work</HostFolder>
      <ReadOnly>false</ReadOnly>
    </MappedFolder>
  </MappedFolders>

  <Networking>Enable</Networking>

  <vGPU>Enable</vGPU>
</Configuration>
```

双击：
```
sandbox.wsb
```
启动后会自动映射
```
D:\work
```
到沙盒环境的桌面上（桌面会存在work文件夹），类似multipass mount。

注意：如果D:\work不存在，沙盒环境拉起失败。

### 1.3 沙盒下映射其他盘符
默认沙盒环境下，只存在C盘，如果想要其他盘符，在沙盒的powershell环境下执行：
```powershell
subst D: C:\Users\WDAGUtilityAccount\Desktop\work
```
这里的work是从配置文件映射d:\work。

## 2. [multipass虚拟机](./Multipass.md)

