# mosquitto使用

## 1. mqtt服务器broker使用的一些注意事项

- 服务启动：`mosquitto.exe -c mosquitto.conf`

- 订阅`$SYS/broker`主题的服务状态不能正常收到数据；

  ```sh
  # mosquitto.conf/sys_interval已经配置了刷新频率，但是订阅主题仍然收不到数据
  # 原因：启用了aclfile.txt访问控制权限限制，里面配置了通用主题，但未配置$SYS/broker主题，该主题是特殊的，不在统配符#下
  
  user buerjia
  topic #
  topic $SYS/#     --- 新加行 ---
  ```

  

- 服务的状态可以使用[`mqtt.fx-1.7.1`](https://cloud.tencent.com/developer/article/1662831)工具订阅，发布消息等和查询broker的状态；

- MQTTAsync_createWithOptions接口的clientId需要保持**全局唯一**，否则被挤掉线时，不会被MQTTAsync_connectionLost感知；



## 2. 主题通配符说明

[Mosquitto 手册页 |Eclipse](https://mosquitto.org/man/mosquitto-8.html)

以主题`a/b/c/d`为例，描述通配符 `+、#`;

- 使用`#`订阅所有主题

  ```sh
  # 订阅所有主题
  mosquitto_sub -t \#
  ```

- 通配符 `+`：

  - 以下订阅将匹配：
    - `a/b/c/d`
    - `+/b/c/d`
    - `a/+/c/d`
    - `a/+/+/d`
    - `+/+/+/+`
  - 以下订阅将不匹配：
    - `a/b/c`
    - `b/+/c/d`
    - `+/+/+`

- 通配符 `#`：

  - 以下订阅将匹配：
    - `a/b/c/d`
    - `#`
    - `a/#`
    - `a/b/#`
    - `a/b/c/#`
    - `+/b/c/#`

**请注意，通配符只能单独使用，因此`a/b+/c`订阅不是对通配符的有效使用。`#`字符只能用作订阅的最后一个字符。**



## 3. 话题设计参考

**发布：**
   `{发送组}/{发送者}/{接收组}/{接收者}/{话题}`
   `{发送组}/{发送者}/{接收组}/{接收者}/{话题}/version`

**订阅：**
   `+/+/{接收组}/{接收者}/{话题}/#`
   `+/+/{接收组}/{接收者}/{话题}/version/#`

**请求（发布）：**
   `{发送组}/{发送者}/{接收组}/{接收者}/{话题_req}`
   `{发送组}/{发送者}/{接收组}/{接收者}/{话题_req}/version`

**回复（订阅）：**
   `+/+/{接收组}/{接收者}/{话题_rsp}/#`
   `+/+/{接收组}/{接收者}/{话题_rsp}/version/#`

**@Note：**

1. 发布/订阅话题时，如果接收组为空，则使用通配符 `+` 表示任意接收组；
2. 请求和回复只在消息主题上做一层特殊的封装处理；
   - 请求方：发布请求 `*_req`，并且订阅回复 `*_rsp`;
   - 回复方：订阅请求 `*_req`，并且发布回复 `*_rsp`;
   - 请求/回复的接口应该有唯一编号，以此来作为请求和回复的关联，回复时，需要使用请求的编号作为响应的唯一编号；
   
     ```c++
     // request示例：
     Client cli(senderGroupName, senderName);
     cli.request(seqNo, topic, message, receieveGroupName, receieverName, [](int code, string response) {
         // async callback request's result code and response data.
     });
     
     // 作为响应的服务方处理请求，并回复数据，seqNo与请求的一致
     cli.response(seqNo, topic, message, receieveGroupName, receieverName);
     ```
   
     

## 4. mosquitto源码分析图

[mosquitto源码分析图来源](https://github.com/hui6075/mosquitto-cluster/blob/master/benchmark/mosquitto_code_analyze.jpg)

![](./img/mosquitto_code_analyze.jpg)

## 5. 补充

[从 HTTP 轮询到 MQTT：我们在 AWS IoT Core 上的架构演进与实战复盘](https://mp.weixin.qq.com/s/3wyrIFf3pQh5EJ0NWbHOjA)

引用文章描述对我来说有用的部分式是关于主题的分层设计方式。

### 5.1 Topic 设计：不只是命名，更是架构

从架构图中不难发现，Topic 是整个消息路由的关键。一个设计良好的 Topic 结构，不仅能让消息路由清晰，还能在权限控制、问题排查甚至成本优化方面带来巨大帮助。

Topic 分为两类：一类是 AWS 保留的系统 Topic（以 `$` 开头，如 `$aws/events/presence/connected/{clientId}` 用于监控设备连接状态），我们只能订阅不能发布；另一类就是与我们业务紧密相关的自定义 Topic。

设计自定义 Topic 是一门艺术。下面结合一个温度传感器的例子，分享一些我们团队总结的最佳实践。

#### 5.1.1 最佳实践一：动静分离，上行下行要分明

一个常见的误区是设备上报和指令下发使用同一个 Topic。更好的做法是使用不同的前缀或路径来区分数据流向。

- 上行数据（Data）

  ：设备 -> 云。建议使用 `/dt` (data) 或 `/up` (upload) 作为前缀。

- 例如

  ：`/dt/sensor/temp-001/report`

- 下行指令（Command）

  ：云 -> 设备。建议使用 `/cmd` (command) 或 `/down` (download) 作为前缀。

- 例如

  ：`/cmd/sensor/temp-001/reboot`

这样做的好处是权限控制可以做得非常精细。例如，你可以配置策略，只允许设备向 `/dt/` 开头的 Topic 发布消息，而只能从 `/cmd/` 开头的 Topic 订阅消息，最小化安全风险。

#### 5.1.2 最佳实践二：层级清晰，包含关键实体信息

Topic 的层级结构应该像一个有意义的 URL。将关键的实体信息（如设备类型、设备 ID）放入 Topic 中，而不是全部塞进消息体（Payload）。

- 推荐结构

  ：`{direction}/{deviceType}/{deviceId}/{action}`

- 好的例子

  ：`/dt/sensor/temp-001/report`

- 不好的例子

  ：`/report` (消息体里是 `{"deviceType": "sensor", "deviceId": "temp-001"}`)

将关键信息放在 Topic 里有两大优势：

1. 路由更高效

   ：你可以使用通配符 (`+` 和 `#`) 对一类设备进行订阅，而无需解析消息体。例如，订阅 `/dt/sensor/#` 可以获取所有传感器的上报数据。

2. Rule 处理更便捷

   ：在 IoT Rule 的 SQL 中，你可以用 `topic(n)` 函数直接从 Topic 路径中提取信息。例如，对于 `/dt/sensor/temp-001/report`，`topic(3)` 的结果就是 `temp-001`。这可以让你在将消息转发到后端服务时，轻松地将设备 ID 等信息带上，避免了复杂的 JSON 解析。

#### 5.1.3 最佳实践三：预留空间，兼顾当下与未来

AWS IoT Core 对 Topic 的层级有数量限制（最多 7 层，总长度不超过 256 字节）。在设计时，既要包含当前业务所需的所有信息，也要为未来的功能扩展预留空间。

`{direction}/{deviceType}/{deviceId}/{action}` 

这样的四层结构通常能满足大部分场景，并且还留有扩展的余地（例如，可以在后面增加版本号或子模块）。

#### 5.1.4 最佳实践四：巧用通配符，实现灵活订阅

MQTT 的通配符是其强大功能之一，要善加利用。

- `+`：
  
   (加号)：单层通配符，可以匹配一个层级。例如 `dt/sensor/+/report` 可以匹配 `dt/sensor/temp-001/report` 和 `dt/sensor/humidity-002/report`，但不能匹配 `dt/sensor/temp-001/extra/report`。

- `#`：
  
   (井号)：多层通配符，必须放在 Topic 的末尾，可以匹配任意数量的后续层级。例如 `dt/sensor/#` 可以匹配上面两个例子以及 `dt/sensor/temp-001/extra/report`。

通过通配符，运维人员或后端服务可以非常灵活地订阅他们关心的消息流，而无需为每个设备单独配置。



## 6. 开启ssl连接

使用脚本生成证书：`./证书/gen_certs.bat`

mosquitto.conf配置中新增：

```
# TLS 监听端口
listener 8883 0.0.0.0

# CA 证书（用来验证客户端）
#cafile D:/MACCURA/5002/mosquitto/ca.crt

# 服务器证书
certfile D:/MACCURA/5002/mosquitto/server.crt

# 服务器私钥
keyfile D:/MACCURA/5002/mosquitto/server.key

# 不验证客户端证书
require_certificate false
```

使用`paho.mqtt.c`通过ssl连接mosquitto：

```c++
MQTTAsync client;
MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer5;
create_opts.MQTTVersion = MQTTVERSION_5;
create_opts.sendWhileDisconnected = 0;

auto startWith = [](const std::string& str, const std::string& pattern) {
    return str.find(pattern) == 0u;
};

// 是否开启ssl连接(如果不开启ssl则不要使用ssl://前缀)
auto host = "ssl://127.0.0.1";
auto port = 8883; // 默认非ssl端口为：1883
auto id = "unique_id";

bool is_ssl = startWith(m_connectOpt.host, "ssl://");

auto uri = fmt::format("{}:{}", host, port);

int rc;
if ((rc = MQTTAsync_createWithOptions(&client, uri.c_str(), id, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts)) != MQTTASYNC_SUCCESS)
{
    return 1; // failed.
}

MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
// conn_opts参数设置
...

MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
ssl_opts.enableServerCertAuth = 0; // 跳过主机名验证

// 开启tls
conn_opts.ssl = &ssl_opts;
```

