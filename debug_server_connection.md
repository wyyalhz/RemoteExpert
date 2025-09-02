# 服务端连接调试指南

## 问题描述
客户端成功发送删除工单请求（消息类型15），但服务端没有收到请求，数据库中工单也没有被删除。

## 已修复的问题

### 1. 消息处理器注册缺失
- **问题**：`MSG_DELETE_WORKORDER` 消息类型没有在服务端注册
- **修复**：在 `NetworkServer::registerMessageHandlers()` 中添加了删除工单消息处理器的注册

### 2. 日志模块支持不足
- **问题**：服务端日志模块缺少删除工单相关的日志方法
- **修复**：在 `NetworkLogger` 中添加了工单操作相关的日志方法

## 调试步骤

### 1. 检查服务端是否正在运行
```bash
# 检查服务端进程
ps aux | grep remote_expert
# 或者
netstat -tlnp | grep 8080
```

### 2. 检查服务端日志
查看服务端控制台输出，应该看到：
```
=== RemoteExpert 服务器启动 ===
版本: 1.0.0
监听地址: 0.0.0.0:8080
数据库路径: data/remote_expert.db
日志级别: INFO
日志文件: logs/server.log
数据库初始化成功
业务服务初始化成功
网络服务器初始化成功
网络服务器启动成功，开始监听连接...
服务器运行中，按 Ctrl+C 退出...
```

### 3. 检查消息处理器注册
服务端启动时应该看到：
```
[Network Server] Message handlers registered successfully
[Network Server] Component connections established
```

### 4. 检查客户端连接
当客户端连接时，服务端应该看到：
```
[Network Server] Server started event received
[Connection Manager] New connection from [客户端IP]:[端口]
```

### 5. 检查消息路由
当收到删除工单请求时，应该看到：
```
[Message Router] Registered handler for message type 15
[Message Router] [客户端IP]:[端口] -> WorkOrderHandler (MSG_DELETE_WORKORDER)
[Work Order Handler] Handling delete work order request
[Work Order Handler] User [用户ID] requesting to delete work order [工单ID] (ticket: [工单编号])
[Work Order Handler] Work order [工单ID] (ticket: [工单编号]) deleted successfully by user [用户ID]
[Network Logger] Work order deleted by [客户端IP]:[端口]: Ticket '[工单编号]', User ID [用户ID]
```

## 可能的问题和解决方案

### 1. 服务端未启动
**症状**：客户端连接失败
**解决**：启动服务端程序

### 2. 端口被占用
**症状**：服务端启动失败，提示端口被占用
**解决**：更换端口或关闭占用端口的程序

### 3. 防火墙阻止
**症状**：客户端无法连接到服务端
**解决**：检查防火墙设置，允许对应端口

### 4. 消息处理器未正确注册
**症状**：服务端收到消息但没有处理
**解决**：检查 `registerMessageHandlers()` 方法是否正确注册了删除工单处理器

### 5. 协议版本不匹配
**症状**：客户端和服务端使用不同的协议版本
**解决**：确保客户端和服务端使用相同的协议定义

## 验证方法

### 1. 重新编译并启动服务端
```bash
cd server
qmake
make
./server -p 8080 -l DEBUG
```

### 2. 重新编译并启动客户端
```bash
cd client
qmake
make
./client
```

### 3. 测试连接
- 客户端连接到服务端
- 尝试删除工单
- 查看服务端日志输出

## 如果问题仍然存在

1. **检查网络连接**：使用 `telnet` 或 `nc` 测试端口连通性
2. **检查服务端状态**：确认服务端正在监听指定端口
3. **检查消息路由**：确认删除工单消息被正确路由到处理器
4. **检查数据库操作**：确认删除操作在数据库层面成功执行
5. **检查权限验证**：确认用户有删除工单的权限

## 技术要点

- **消息类型**：`MSG_DELETE_WORKORDER = 15`
- **处理器注册**：在 `NetworkServer::registerMessageHandlers()` 中注册
- **消息路由**：通过 `MessageRouter` 路由到 `WorkOrderHandler`
- **日志记录**：使用 `NetworkLogger::workOrderDeleted()` 记录删除操作
