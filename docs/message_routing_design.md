# 消息路由器设计文档

## 1. 概述

MessageRouter是网络通信层的核心组件，负责根据消息类型将接收到的消息分发到对应的ProtocolHandler进行处理。它采用了**策略模式**和**观察者模式**，实现了高度解耦的消息处理架构。

## 2. 架构设计

### 2.1 核心组件关系
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   TCPServer     │───▶│ConnectionManager│───▶│ MessageRouter   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                       │
                                                       ▼
                       ┌─────────────────────────────────────────┐
                       │           ProtocolHandlers              │
                       ├─────────────────┬───────────────────────┤
                       │   UserHandler   │   WorkOrderHandler    │
                       ├─────────────────┼───────────────────────┤
                       │   ChatHandler   │   VideoHandler        │
                       ├─────────────────┼───────────────────────┤
                       │ ControlHandler  │   SystemHandler       │
                       └─────────────────┴───────────────────────┘
```

### 2.2 消息处理流程
```
1. 客户端发送消息
   ↓
2. TCPServer接收TCP数据
   ↓
3. ConnectionManager调用drainPackets()拆包
   ↓
4. 获取Packet对象，包含type、json、bin
   ↓
5. MessageRouter根据packet.type查找对应的Handler
   ↓
6. 调用Handler.handleMessage(socket, packet)
   ↓
7. Handler处理业务逻辑并返回响应
```

## 3. 消息类型映射

### 3.1 认证类消息 (1-9)
| 消息类型 | 处理器 | 说明 |
|---------|--------|------|
| MSG_REGISTER (1) | UserHandler | 用户注册 |
| MSG_LOGIN (2) | UserHandler | 用户登录 |
| MSG_LOGOUT (3) | UserHandler | 用户登出 |
| MSG_HEARTBEAT (4) | SystemHandler | 心跳包 |

### 3.2 工单类消息 (10-19)
| 消息类型 | 处理器 | 说明 |
|---------|--------|------|
| MSG_CREATE_WORKORDER (10) | WorkOrderHandler | 创建工单 |
| MSG_JOIN_WORKORDER (11) | WorkOrderHandler | 加入工单 |
| MSG_LEAVE_WORKORDER (12) | WorkOrderHandler | 离开工单 |
| MSG_UPDATE_WORKORDER (13) | WorkOrderHandler | 更新工单 |
| MSG_LIST_WORKORDERS (14) | WorkOrderHandler | 获取工单列表 |

### 3.3 聊天类消息 (20-29)
| 消息类型 | 处理器 | 说明 |
|---------|--------|------|
| MSG_TEXT (20) | ChatHandler | 文本消息 |
| MSG_DEVICE_DATA (21) | ChatHandler | 设备数据 |
| MSG_FILE_TRANSFER (22) | ChatHandler | 文件传输 |
| MSG_SCREENSHOT (23) | ChatHandler | 截图 |

### 3.4 音视频类消息 (30-49)
| 消息类型 | 处理器 | 说明 |
|---------|--------|------|
| MSG_VIDEO_FRAME (30) | VideoHandler | 视频帧 |
| MSG_AUDIO_FRAME (31) | AudioHandler | 音频帧 |
| MSG_VIDEO_CONTROL (32) | VideoHandler | 视频控制 |
| MSG_AUDIO_CONTROL (33) | AudioHandler | 音频控制 |

### 3.5 控制类消息 (50-59)
| 消息类型 | 处理器 | 说明 |
|---------|--------|------|
| MSG_CONTROL (50) | ControlHandler | 通用控制 |
| MSG_DEVICE_CONTROL (51) | ControlHandler | 设备控制 |
| MSG_SYSTEM_CONTROL (52) | SystemHandler | 系统控制 |

### 3.6 系统类消息 (90-99)
| 消息类型 | 处理器 | 说明 |
|---------|--------|------|
| MSG_SERVER_EVENT (90) | SystemHandler | 服务器事件 |
| MSG_ERROR (91) | SystemHandler | 错误消息 |
| MSG_NOTIFICATION (92) | SystemHandler | 通知消息 |

## 4. 核心实现

### 4.1 MessageRouter类结构
```cpp
class MessageRouter : public QObject
{
    Q_OBJECT
public:
    // 注册/注销处理器
    void registerHandler(quint16 msgType, ProtocolHandler* handler);
    void unregisterHandler(quint16 msgType);
    
    // 消息处理
    void handleMessage(QTcpSocket* socket, const Packet& packet);
    
    // 查询功能
    ProtocolHandler* getHandler(quint16 msgType) const;
    QList<quint16> getRegisteredMessageTypes() const;

private:
    QHash<quint16, ProtocolHandler*> handlers_;  // 消息类型到处理器的映射
    
    void logUnhandledMessage(quint16 msgType, QTcpSocket* socket);
};
```

### 4.2 消息处理核心逻辑
```cpp
void MessageRouter::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    // 1. 查找对应的处理器
    ProtocolHandler* handler = handlers_.value(packet.type, nullptr);
    
    if (handler) {
        // 2. 调用处理器处理消息
        handler->handleMessage(socket, packet);
    } else {
        // 3. 记录未处理的消息
        logUnhandledMessage(packet.type, socket);
        
        // 4. 发送错误响应
        QJsonObject errorData = MessageBuilder::buildErrorResponse(
            404, 
            QString("Unknown message type: %1").arg(packet.type)
        );
        
        QByteArray response = buildPacket(MSG_ERROR, errorData);
        socket->write(response);
    }
}
```

## 5. 处理器注册机制

### 5.1 注册流程
```cpp
void NetworkServer::registerMessageHandlers()
{
    // 认证类消息
    messageRouter_->registerHandler(MSG_LOGIN, userHandler_);
    messageRouter_->registerHandler(MSG_REGISTER, userHandler_);
    messageRouter_->registerHandler(MSG_LOGOUT, userHandler_);
    
    // 工单类消息
    messageRouter_->registerHandler(MSG_CREATE_WORKORDER, workOrderHandler_);
    messageRouter_->registerHandler(MSG_JOIN_WORKORDER, workOrderHandler_);
    messageRouter_->registerHandler(MSG_LEAVE_WORKORDER, workOrderHandler_);
    
    // 聊天类消息
    messageRouter_->registerHandler(MSG_TEXT, chatHandler_);
    messageRouter_->registerHandler(MSG_DEVICE_DATA, chatHandler_);
    messageRouter_->registerHandler(MSG_FILE_TRANSFER, chatHandler_);
    
    // 音视频类消息
    messageRouter_->registerHandler(MSG_VIDEO_FRAME, videoHandler_);
    messageRouter_->registerHandler(MSG_AUDIO_FRAME, audioHandler_);
    
    // 控制类消息
    messageRouter_->registerHandler(MSG_CONTROL, controlHandler_);
    messageRouter_->registerHandler(MSG_DEVICE_CONTROL, controlHandler_);
    
    // 系统类消息
    messageRouter_->registerHandler(MSG_SERVER_EVENT, systemHandler_);
    messageRouter_->registerHandler(MSG_ERROR, systemHandler_);
}
```

### 5.2 动态注册支持
```cpp
// 支持运行时动态注册新的消息处理器
void MessageRouter::registerHandler(quint16 msgType, ProtocolHandler* handler)
{
    if (handler) {
        handlers_[msgType] = handler;
        qDebug() << "Registered handler for message type:" << msgType;
    }
}

void MessageRouter::unregisterHandler(quint16 msgType)
{
    if (handlers_.remove(msgType) > 0) {
        qDebug() << "Unregistered handler for message type:" << msgType;
    }
}
```

## 6. 错误处理机制

### 6.1 未处理消息
```cpp
void MessageRouter::logUnhandledMessage(quint16 msgType, QTcpSocket* socket)
{
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    
    qWarning() << "Unhandled message type:" << msgType 
               << "from client:" << clientInfo;
    
    // 记录到日志系统
    NetworkLogger::warning("Message Router", 
                          QString("Unhandled message type %1 from %2")
                          .arg(msgType).arg(clientInfo));
}
```

### 6.2 处理器异常处理
```cpp
void MessageRouter::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    ProtocolHandler* handler = handlers_.value(packet.type, nullptr);
    
    if (handler) {
        try {
            handler->handleMessage(socket, packet);
        } catch (const std::exception& e) {
            // 处理器异常，发送错误响应
            QJsonObject errorData = MessageBuilder::buildErrorResponse(
                500, 
                QString("Handler error: %1").arg(e.what())
            );
            
            QByteArray response = buildPacket(MSG_ERROR, errorData);
            socket->write(response);
            
            qCritical() << "Handler exception for message type:" << packet.type 
                       << "Error:" << e.what();
        }
    } else {
        logUnhandledMessage(packet.type, socket);
    }
}
```

## 7. 性能优化

### 7.1 处理器缓存
- 使用QHash提供O(1)的查找性能
- 避免重复的字符串比较
- 支持快速的消息类型验证

### 7.2 内存管理
- 处理器对象由NetworkServer统一管理
- 避免重复创建和销毁处理器
- 使用智能指针管理处理器生命周期

### 7.3 并发处理
- 每个连接在独立线程中处理
- 处理器内部支持异步操作
- 避免阻塞主线程

## 8. 扩展性设计

### 8.1 新增消息类型
1. 在MsgType枚举中添加新的消息类型
2. 创建对应的ProtocolHandler
3. 在NetworkServer中注册新的处理器
4. 更新协议文档

### 8.2 处理器插件化
```cpp
// 支持插件化的处理器注册
class PluginHandler : public ProtocolHandler
{
public:
    virtual QString getPluginName() const = 0;
    virtual QStringList getSupportedMessageTypes() const = 0;
};

// 动态加载插件处理器
void MessageRouter::loadPluginHandler(PluginHandler* plugin)
{
    QStringList messageTypes = plugin->getSupportedMessageTypes();
    for (const QString& typeStr : messageTypes) {
        quint16 msgType = typeStr.toUShort();
        registerHandler(msgType, plugin);
    }
}
```

## 9. 监控和调试

### 9.1 消息统计
```cpp
class MessageRouter : public QObject
{
private:
    QHash<quint16, int> messageCounts_;  // 消息类型统计
    QHash<quint16, qint64> messageLatency_;  // 消息处理延迟
    
public:
    // 获取消息统计信息
    QJsonObject getMessageStatistics() const;
    
    // 重置统计信息
    void resetStatistics();
};
```

### 9.2 调试支持
```cpp
// 启用详细日志
void MessageRouter::setDebugMode(bool enabled)
{
    debugMode_ = enabled;
    if (debugMode_) {
        qDebug() << "Message Router debug mode enabled";
    }
}

// 消息处理日志
void MessageRouter::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    if (debugMode_) {
        qDebug() << "Processing message type:" << packet.type
                 << "from:" << socket->peerAddress().toString();
    }
    
    // ... 处理逻辑
}
```

## 10. 测试策略

### 10.1 单元测试
- 测试消息类型映射
- 测试处理器注册/注销
- 测试错误处理机制

### 10.2 集成测试
- 测试完整的消息处理流程
- 测试多客户端并发
- 测试异常情况处理

### 10.3 性能测试
- 测试消息处理吞吐量
- 测试内存使用情况
- 测试并发连接数限制
