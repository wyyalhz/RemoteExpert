# 工单详情获取功能修复测试文档

## 修复内容概述

本次修复解决了工单详细信息查看异常的问题，主要包括：

1. 添加了获取工单详情的消息类型 `MSG_GET_WORKORDER = 16`
2. 实现了完整的工单详情获取流程
3. 修复了用户信息传递缺失的问题
4. 完善了服务器端响应处理

## 测试步骤

### 1. 客户端测试

#### 1.1 编译测试
```bash
cd client
qmake
make
```

#### 1.2 功能测试
1. 启动客户端应用
2. 登录系统（工厂端或专家端）
3. 进入工单列表页面
4. 点击任意工单的"进入"按钮
5. 验证是否成功打开工单详情对话框
6. 检查工单详情信息是否正确显示

#### 1.3 网络请求验证
使用网络抓包工具（如Wireshark）验证：
- 客户端发送的消息类型是否为 `MSG_GET_WORKORDER (16)`
- 消息内容是否包含：
  - `workorderId`: 工单ID
  - `userId`: 当前用户ID
  - `userType`: 用户类型（0=工厂端，1=专家端）
  - `timestamp`: 时间戳

### 2. 服务器端测试

#### 2.1 消息处理测试
验证服务器是否正确处理 `MSG_GET_WORKORDER` 消息：
1. 解析请求参数
2. 根据工单ID查询工单信息
3. 根据 `creator_id` 查询工厂端用户名（usertype=0）
4. 根据 `assigned_to` 查询专家端用户名（usertype=1）
5. 返回包含以下字段的响应：
   - `ticketid`: 工单ID
   - `status`: 工单状态
   - `factory_username`: 工厂端用户名
   - `expert_username`: 专家端用户名
   - `title`: 工单标题
   - `description`: 工单描述

#### 2.2 错误处理测试
测试以下错误情况：
1. 工单ID不存在
2. 用户ID无效
3. 用户类型不匹配
4. 数据库连接异常

### 3. 数据流验证

#### 3.1 客户端数据流
```
用户点击"进入"按钮
    ↓
TicketPage::showTicketDetail()
    ↓
获取当前用户ID和用户类型
    ↓
TicketService::getTicketDetail()
    ↓
NetworkClient::sendGetTicketDetailRequest()
    ↓
发送 MSG_GET_WORKORDER 消息
```

#### 3.2 服务器响应处理
```
NetworkClient::onMessageReceived()
    ↓
case MSG_GET_WORKORDER:
    ↓
emit getTicketDetailResponse()
    ↓
TicketService::onGetTicketDetailResponse()
    ↓
解析响应数据
    ↓
emit ticketDetailReceived()
    ↓
TicketPage::onTicketDetailReceived()
    ↓
创建并显示TicketDialogDetail
```

## 预期结果

### 成功情况
1. 点击"进入"按钮后，成功获取工单详情
2. 工单详情对话框正确显示所有字段
3. 工厂端用户名和专家端用户名正确显示
4. 网络请求和响应日志正常记录

### 失败情况
1. 网络异常时显示错误提示
2. 工单不存在时显示相应错误信息
3. 用户权限不足时显示权限错误

## 注意事项

1. 确保客户端和服务器端的消息类型定义一致
2. 验证用户认证状态和权限检查
3. 检查网络连接的稳定性
4. 监控日志输出，确保错误处理正常

## 相关文件

### 新增/修改的客户端文件
- `common/protocol/types/enums.h` - 添加消息类型
- `common/protocol/builders/message_builder.h/cpp` - 添加消息构建方法
- `client/Network/client/network_client.h/cpp` - 添加网络请求方法
- `client/Business/services/TicketService.h/cpp` - 添加工单详情获取逻辑
- `client/Presentation/pages/TicketPage/ticket_page.h/cpp` - 添加详情显示逻辑
- `client/Business/models/Ticket.cpp` - 增强数据解析支持

### 服务器端需要实现
- 处理 `MSG_GET_WORKORDER` 消息
- 查询工单详细信息
- 查询用户信息（工厂端和专家端）
- 返回标准格式的响应数据
