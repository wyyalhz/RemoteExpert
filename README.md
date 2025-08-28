# Remote Expert Skeleton (Qt/qmake, C++11)

最小可运行项目骨架：**服务器 + 工厂端 + 专家端**，统一协议已内置。

## 要求环境（Ubuntu 18.04）
```bash
sudo apt update
sudo apt install -y build-essential qt5-default qtcreator qtmultimedia5-dev \
                    qml-module-qtquick-controls2 qml-module-qtcharts \
                    libqt5charts5-dev libsqlite3-dev
```

> 本骨架当前只跑通 **文本聊天**（MSG_TEXT）与 **加入工单**（MSG_JOIN_WORKORDER）。
> 设备数据/音视频后续直接按 `MsgType` 扩展即可。

## 目录结构
```
remote-expert-skeleton/
  common/            # 统一协议（protocol.h/.cpp）——三端共享
  server/            # 服务器（控制台程序）
  client-factory/    # 工厂端（Qt Widgets）
  client-expert/     # 专家端（Qt Widgets）
```

## 构建
用 Qt Creator 打开对应 `.pro` 即可；或命令行：

### 构建并运行服务器
```bash
cd server && qmake && make -j && ./server -p 9000
```
### 构建并运行客户端（工厂端 / 专家端）
分别在 `client-factory`、`client-expert` 目录：
```bash
qmake && make -j && ./client-factory
qmake && make -j && ./client-expert
```

## 使用方法（最小演示）
1. 先启动服务器：`./server -p 9000`
2. 打开两个客户端（工厂端 & 专家端）
3. 在两端 UI：
   - Host: `127.0.0.1`，Port: `9000`，点击“连接”
   - User 任填（默认工厂端`factory-A`，专家端`expert-B`）
   - RoomId 一致（默认 `R123`），点击“加入工单”
   - 文本框输入消息，点“发送文本”——另一端可收到

## 扩展开发指引
- **协议**：见 `common/protocol.h`，新增类型时往 `enum MsgType` 里追加值，并约定 JSON 字段；
  发送使用 `buildPacket()`，接收通过 `drainPackets()` 拆包。
- **服务器**：当前 `RoomHub` 只做转发（按房间广播）。后续可增加认证、SQLite记录等。
- **客户端**：`ClientConn` 封装了 TCP + 拆包，UI 尽量通过信号槽解耦。

## 常见问题
- 若 `qmake` 报错缺少模块，请确认已安装 `qt5-default`、`qtbase5-dev` 等基础包。
- 同机测试语音/视频时请佩戴耳机避免啸叫（音视频功能开发时适用）。