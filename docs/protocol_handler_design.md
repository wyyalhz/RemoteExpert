# 协议处理器设计文档

## 1. 概述

ProtocolHandler是消息处理的核心抽象基类，定义了所有具体协议处理器必须实现的接口。它采用了**模板方法模式**和**策略模式**，提供了统一的消息处理框架。

## 2. 架构设计

### 2.1 类层次结构
```
                    ProtocolHandler (抽象基类)
                           │
                           ├── UserHandler
                           ├── WorkOrderHandler
                           ├── ChatHandler
                           ├── VideoHandler
                           ├── AudioHandler
                           ├── ControlHandler
                           └── SystemHandler
```

### 2.2 核心组件关系
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ MessageRouter   │───▶│ProtocolHandler  │───▶│ BusinessService │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                              │
                              ▼
                       ┌─────────────────┐
                       │ConnectionManager│
                       └─────────────────┘
```

## 3. 基类设计

### 3.1 ProtocolHandler基类
```cpp
class ProtocolHandler : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolHandler(QObject *parent = nullptr);
    virtual ~ProtocolHandler();

    // 核心消息处理接口
    virtual void handleMessage(QTcpSocket* socket, const Packet& packet) = 0;
    
    // 设置连接管理器
    void setConnectionManager(ConnectionManager* manager);
    
    // 获取处理器名称
    virtual QString getHandlerName() const = 0;
    
    // 获取支持的消息类型
    virtual QList<quint16> getSupportedMessageTypes() const = 0;

protected:
    // 连接管理器访问
    ConnectionManager* getConnectionManager() const;
    
    // 客户端上下文访问
    ClientContext* getClientContext(QTcpSocket* socket) const;
    
    // 认证检查
    bool checkAuthentication(QTcpSocket* socket);
    bool checkRoomMembership(QTcpSocket* socket);
    
    // 消息发送
    void sendToClient(QTcpSocket* socket, const QByteArray& data);
    void sendToClient(const QString& username, const QByteArray& data);
    void broadcastToRoom(QTcpSocket* socket, const Packet& packet);
    
    // 响应构建
    void sendSuccessResponse(QTcpSocket* socket, const QString& message, 
                           const QJsonObject& data = QJsonObject());
    void sendErrorResponse(QTcpSocket* socket, int code, const QString& message,
                          const QJsonObject& data = QJsonObject());
    
    // 消息验证
    bool validateMessage(const QJsonObject& data, QString& error);
    
    // 日志记录
    void logMessage(QTcpSocket* socket, const QString& action, const QString& details = QString());

private:
    ConnectionManager* connectionManager_;
    
    // 模板方法：消息处理流程
    void processMessage(QTcpSocket* socket, const Packet& packet);
    
    // 钩子方法
    virtual bool preProcess(QTcpSocket* socket, const Packet& packet);
    virtual void postProcess(QTcpSocket* socket, const Packet& packet);
};
```

### 3.2 模板方法模式实现
```cpp
void ProtocolHandler::processMessage(QTcpSocket* socket, const Packet& packet)
{
    // 1. 预处理检查
    if (!preProcess(socket, packet)) {
        return;
    }
    
    // 2. 核心消息处理
    handleMessage(socket, packet);
    
    // 3. 后处理
    postProcess(socket, packet);
}

bool ProtocolHandler::preProcess(QTcpSocket* socket, const Packet& packet)
{
    // 默认实现：检查认证状态
    if (!checkAuthentication(socket)) {
        sendErrorResponse(socket, 401, "Authentication required");
        return false;
    }
    return true;
}

void ProtocolHandler::postProcess(QTcpSocket* socket, const Packet& packet)
{
    // 默认实现：更新最后活动时间
    if (connectionManager_) {
        connectionManager_->updateLastActivity(socket);
    }
}
```

## 4. 具体处理器实现

### 4.1 UserHandler示例
```cpp
class UserHandler : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit UserHandler(UserService* userService, QObject *parent = nullptr);
    ~UserHandler();

    // 实现基类接口
    void handleMessage(QTcpSocket* socket, const Packet& packet) override;
    QString getHandlerName() const override { return "UserHandler"; }
    QList<quint16> getSupportedMessageTypes() const override;

protected:
    // 重写钩子方法
    bool preProcess(QTcpSocket* socket, const Packet& packet) override;

private:
    UserService* userService_;
    
    // 具体消息处理方法
    void handleLogin(QTcpSocket* socket, const QJsonObject& data);
    void handleRegister(QTcpSocket* socket, const QJsonObject& data);
    void handleLogout(QTcpSocket* socket, const QJsonObject& data);
    
    // 辅助方法
    void updateClientAuthentication(QTcpSocket* socket, const QString& username, bool authenticated);
};

QList<quint16> UserHandler::getSupportedMessageTypes() const
{
    return {MSG_LOGIN, MSG_REGISTER, MSG_LOGOUT};
}

bool UserHandler::preProcess(QTcpSocket* socket, const Packet& packet)
{
    // 登录和注册消息不需要认证检查
    if (packet.type == MSG_LOGIN || packet.type == MSG_REGISTER) {
        return true;
    }
    
    // 其他消息需要认证
    return ProtocolHandler::preProcess(socket, packet);
}

void UserHandler::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    switch (packet.type) {
        case MSG_LOGIN:
            handleLogin(socket, packet.json);
            break;
        case MSG_REGISTER:
            handleRegister(socket, packet.json);
            break;
        case MSG_LOGOUT:
            handleLogout(socket, packet.json);
            break;
        default:
            sendErrorResponse(socket, 404, 
                QString("Unknown user message type: %1").arg(packet.type));
            break;
    }
}
```

### 4.2 ChatHandler示例
```cpp
class ChatHandler : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit ChatHandler(QObject *parent = nullptr);
    ~ChatHandler();

    void handleMessage(QTcpSocket* socket, const Packet& packet) override;
    QString getHandlerName() const override { return "ChatHandler"; }
    QList<quint16> getSupportedMessageTypes() const override;

protected:
    bool preProcess(QTcpSocket* socket, const Packet& packet) override;

private:
    void handleTextMessage(QTcpSocket* socket, const Packet& packet);
    void handleDeviceData(QTcpSocket* socket, const Packet& packet);
    void handleFileTransfer(QTcpSocket* socket, const Packet& packet);
    void handleScreenshot(QTcpSocket* socket, const Packet& packet);
    
    // 消息验证
    bool validateTextMessage(const QJsonObject& data, QString& error);
    bool validateDeviceData(const QJsonObject& data, QString& error);
};

QList<quint16> ChatHandler::getSupportedMessageTypes() const
{
    return {MSG_TEXT, MSG_DEVICE_DATA, MSG_FILE_TRANSFER, MSG_SCREENSHOT};
}

bool ChatHandler::preProcess(QTcpSocket* socket, const Packet& packet)
{
    // 聊天消息需要认证和房间成员身份
    if (!ProtocolHandler::preProcess(socket, packet)) {
        return false;
    }
    
    return checkRoomMembership(socket);
}

void ChatHandler::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    switch (packet.type) {
        case MSG_TEXT:
            handleTextMessage(socket, packet);
            break;
        case MSG_DEVICE_DATA:
            handleDeviceData(socket, packet);
            break;
        case MSG_FILE_TRANSFER:
            handleFileTransfer(socket, packet);
            break;
        case MSG_SCREENSHOT:
            handleScreenshot(socket, packet);
            break;
        default:
            sendErrorResponse(socket, 404, 
                QString("Unknown chat message type: %1").arg(packet.type));
            break;
    }
}
```

## 5. 消息处理流程

### 5.1 标准处理流程
```
1. MessageRouter调用processMessage()
   ↓
2. 执行preProcess()预处理
   ├── 认证检查
   ├── 权限验证
   └── 参数验证
   ↓
3. 调用handleMessage()核心处理
   ├── 消息类型分发
   ├── 业务逻辑处理
   └── 响应构建
   ↓
4. 执行postProcess()后处理
   ├── 状态更新
   ├── 日志记录
   └── 清理工作
```

### 5.2 错误处理流程
```
1. 预处理失败
   ├── 发送错误响应
   └── 记录错误日志
   ↓
2. 消息验证失败
   ├── 发送参数错误响应
   └── 记录验证失败日志
   ↓
3. 业务处理异常
   ├── 捕获异常
   ├── 发送服务器错误响应
   └── 记录异常日志
```

## 6. 认证和权限控制

### 6.1 认证检查
```cpp
bool ProtocolHandler::checkAuthentication(QTcpSocket* socket)
{
    ClientContext* context = getClientContext(socket);
    if (!context || !context->isAuthenticated) {
        return false;
    }
    return true;
}
```

### 6.2 房间成员身份检查
```cpp
bool ProtocolHandler::checkRoomMembership(QTcpSocket* socket)
{
    ClientContext* context = getClientContext(socket);
    if (!context || context->currentRoom.isEmpty()) {
        return false;
    }
    return true;
}
```

### 6.3 权限验证
```cpp
bool ProtocolHandler::checkPermission(QTcpSocket* socket, const QString& permission)
{
    ClientContext* context = getClientContext(socket);
    if (!context) {
        return false;
    }
    
    // 根据用户类型和角色检查权限
    if (permission == "device_control" && context->userType != USER_TYPE_EXPERT) {
        return false;
    }
    
    return true;
}
```

## 7. 消息发送机制

### 7.1 单播消息
```cpp
void ProtocolHandler::sendToClient(QTcpSocket* socket, const QByteArray& data)
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(data);
        logMessage(socket, "Message sent", QString("Size: %1 bytes").arg(data.size()));
    }
}
```

### 7.2 广播消息
```cpp
void ProtocolHandler::broadcastToRoom(QTcpSocket* socket, const Packet& packet)
{
    ClientContext* context = getClientContext(socket);
    if (!context || context->currentRoom.isEmpty()) {
        return;
    }
    
    QByteArray data = buildPacket(packet.type, packet.json, packet.bin);
    if (connectionManager_) {
        connectionManager_->broadcastToRoom(context->currentRoom, data, socket);
    }
}
```

### 7.3 响应构建
```cpp
void ProtocolHandler::sendSuccessResponse(QTcpSocket* socket, const QString& message, 
                                         const QJsonObject& data)
{
    QJsonObject response = MessageBuilder::buildSuccessResponse(message, data);
    QByteArray packet = buildPacket(MSG_SERVER_EVENT, response);
    sendToClient(socket, packet);
}

void ProtocolHandler::sendErrorResponse(QTcpSocket* socket, int code, 
                                       const QString& message, const QJsonObject& data)
{
    QJsonObject response = MessageBuilder::buildErrorResponse(code, message, data);
    QByteArray packet = buildPacket(MSG_ERROR, response);
    sendToClient(socket, packet);
}
```

## 8. 日志和监控

### 8.1 消息日志
```cpp
void ProtocolHandler::logMessage(QTcpSocket* socket, const QString& action, 
                                const QString& details)
{
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    
    ClientContext* context = getClientContext(socket);
    QString username = context ? context->username : "unknown";
    
    QString logMessage = QString("[%1] %2 - %3")
                        .arg(username)
                        .arg(action)
                        .arg(details);
    
    NetworkLogger::info(getHandlerName(), logMessage);
}
```

### 8.2 性能监控
```cpp
class ProtocolHandler : public QObject
{
private:
    QElapsedTimer processingTimer_;
    QHash<quint16, qint64> processingTimes_;
    
protected:
    void startProcessingTimer()
    {
        processingTimer_.start();
    }
    
    void endProcessingTimer(quint16 msgType)
    {
        qint64 elapsed = processingTimer_.elapsed();
        processingTimes_[msgType] = elapsed;
        
        if (elapsed > 100) {  // 超过100ms记录警告
            logMessage(nullptr, "Slow processing", 
                      QString("Message type %1 took %2ms").arg(msgType).arg(elapsed));
        }
    }
};
```

## 9. 扩展性设计

### 9.1 插件化处理器
```cpp
class PluginProtocolHandler : public ProtocolHandler
{
public:
    virtual QString getPluginName() const = 0;
    virtual QString getPluginVersion() const = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
};

// 插件管理器
class HandlerPluginManager : public QObject
{
public:
    void loadPlugin(const QString& pluginPath);
    void unloadPlugin(const QString& pluginName);
    QList<PluginProtocolHandler*> getLoadedPlugins() const;
};
```

### 9.2 中间件支持
```cpp
class MessageMiddleware
{
public:
    virtual bool process(QTcpSocket* socket, Packet& packet) = 0;
    virtual QString getName() const = 0;
};

class ProtocolHandler : public QObject
{
private:
    QList<MessageMiddleware*> middlewares_;
    
public:
    void addMiddleware(MessageMiddleware* middleware);
    void removeMiddleware(MessageMiddleware* middleware);
    
protected:
    bool processThroughMiddlewares(QTcpSocket* socket, Packet& packet);
};
```

## 10. 测试策略

### 10.1 单元测试
```cpp
class ProtocolHandlerTest : public QObject
{
    Q_OBJECT
    
private slots:
    void testAuthenticationCheck();
    void testMessageValidation();
    void testResponseBuilding();
    void testErrorHandling();
    void testPerformance();
};
```

### 10.2 集成测试
```cpp
class HandlerIntegrationTest : public QObject
{
    Q_OBJECT
    
private slots:
    void testCompleteMessageFlow();
    void testConcurrentMessageHandling();
    void testErrorRecovery();
    void testMemoryLeaks();
};
```

### 10.3 性能测试
```cpp
class HandlerPerformanceTest : public QObject
{
    Q_OBJECT
    
private slots:
    void testMessageThroughput();
    void testMemoryUsage();
    void testConcurrentConnections();
    void testLatency();
};
```
