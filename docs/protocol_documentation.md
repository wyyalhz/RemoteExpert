# RemoteExpert 网络通信协议文档

## 1. 协议概述

### 1.1 协议结构
RemoteExpert采用基于TCP的二进制协议，消息格式如下：
```
[uint32 length][uint16 type][uint32 jsonSize][jsonBytes][bin...]
```

- **length**: 从type开始的总字节数（大端序）
- **type**: 消息类型（见MsgType枚举）
- **jsonSize**: JSON字节长度（UTF-8, Compact格式）
- **jsonBytes**: JSON数据（包含业务字段）
- **bin**: 可选二进制负载（如JPEG/PCM等）

### 1.2 消息类型分类
- **认证类** (1-9): 用户注册、登录等
- **聊天类** (10-19): 文本消息、设备数据等
- **工单类** (20-29): 工单创建、加入等
- **音视频类** (30-49): 视频帧、音频帧等
- **控制类** (50-59): 控制指令等
- **系统类** (90-99): 服务器事件、错误等

## 2. 认证类消息

### 2.1 用户注册 (MSG_REGISTER = 1)

**请求格式:**
```json
{
    "username": "string",     // 用户名（必填）
    "password": "string",     // 密码（必填）
    "email": "string",        // 邮箱（可选）
    "phone": "string",        // 手机号（可选）
    "user_type": 0            // 用户类型：0-普通用户，1-专家
}
```

**成功响应:**
```json
{
    "code": 200,
    "message": "Registration successful",
    "data": {
        "username": "string",
        "user_type": 0
    }
}
```

**错误响应:**
```json
{
    "code": 400,
    "message": "Username already exists",
    "data": null
}
```

### 2.2 用户登录 (MSG_LOGIN = 2)

**请求格式:**
```json
{
    "username": "string",     // 用户名（必填）
    "password": "string",     // 密码（必填）
    "user_type": 0            // 用户类型：0-普通用户，1-专家
}
```

**成功响应:**
```json
{
    "code": 200,
    "message": "Login successful",
    "data": {
        "username": "string",
        "user_type": 0,
        "session_id": "string"
    }
}
```

**错误响应:**
```json
{
    "code": 401,
    "message": "Invalid username or password",
    "data": null
}
```

## 3. 工单类消息

### 3.1 创建工单 (MSG_CREATE_WORKORDER = 3)

**请求格式:**
```json
{
    "title": "string",        // 工单标题（必填）
    "description": "string",  // 工单描述（必填）
    "priority": 1,            // 优先级：1-低，2-中，3-高
    "category": "string",     // 工单分类
    "device_info": {          // 设备信息（可选）
        "device_type": "string",
        "device_model": "string",
        "serial_number": "string"
    }
}
```

**成功响应:**
```json
{
    "code": 200,
    "message": "Work order created successfully",
    "data": {
        "workorder_id": "string",
        "room_id": "string",
        "status": "created"
    }
}
```

### 3.2 加入工单 (MSG_JOIN_WORKORDER = 4)

**请求格式:**
```json
{
    "workorder_id": "string", // 工单ID（必填）
    "role": "string"          // 角色：creator/expert/observer
}
```

**成功响应:**
```json
{
    "code": 200,
    "message": "Joined work order successfully",
    "data": {
        "room_id": "string",
        "workorder_info": {
            "id": "string",
            "title": "string",
            "status": "string",
            "members": [
                {
                    "username": "string",
                    "role": "string",
                    "online": true
                }
            ]
        }
    }
}
```

## 4. 聊天类消息

### 4.1 文本消息 (MSG_TEXT = 10)

**请求格式:**
```json
{
    "room_id": "string",      // 房间ID（必填）
    "text": "string",         // 消息内容（必填）
    "ts": 1234567890,         // 时间戳（必填）
    "message_id": "string"    // 消息ID（可选）
}
```

**广播格式:**
```json
{
    "room_id": "string",
    "text": "string",
    "ts": 1234567890,
    "sender": "string",       // 发送者用户名
    "message_id": "string"
}
```

### 4.2 设备数据 (MSG_DEVICE_DATA = 20)

**请求格式:**
```json
{
    "room_id": "string",      // 房间ID（必填）
    "device_type": "string",  // 设备类型
    "data": {                 // 设备数据
        "temperature": 25.5,
        "humidity": 60.0,
        "status": "normal"
    },
    "ts": 1234567890          // 时间戳
}
```

## 5. 音视频类消息

### 5.1 视频帧 (MSG_VIDEO_FRAME = 30)

**请求格式:**
```json
{
    "room_id": "string",      // 房间ID（必填）
    "frame_id": "string",     // 帧ID
    "width": 1920,            // 视频宽度
    "height": 1080,           // 视频高度
    "fps": 30,                // 帧率
    "ts": 1234567890          // 时间戳
}
```
**二进制数据:** JPEG格式的视频帧数据

### 5.2 音频帧 (MSG_AUDIO_FRAME = 40)

**请求格式:**
```json
{
    "room_id": "string",      // 房间ID（必填）
    "frame_id": "string",     // 帧ID
    "sample_rate": 44100,     // 采样率
    "channels": 2,            // 声道数
    "ts": 1234567890          // 时间戳
}
```
**二进制数据:** PCM S16LE格式的音频数据

## 6. 控制类消息

### 6.1 控制指令 (MSG_CONTROL = 50)

**请求格式:**
```json
{
    "room_id": "string",      // 房间ID（必填）
    "control_type": "string", // 控制类型：start_video/stop_video/start_audio/stop_audio
    "target": "string",       // 目标用户（可选，为空表示广播）
    "params": {               // 控制参数
        "quality": "high",
        "bitrate": 1000000
    },
    "ts": 1234567890          // 时间戳
}
```

## 7. 系统类消息

### 7.1 服务器事件 (MSG_SERVER_EVENT = 90)

**服务器推送格式:**
```json
{
    "event_type": "string",   // 事件类型：user_joined/user_left/room_closed/error
    "room_id": "string",      // 房间ID
    "data": {                 // 事件数据
        "username": "string",
        "message": "string"
    },
    "ts": 1234567890          // 时间戳
}
```

## 8. 错误码定义

| 错误码 | 说明 |
|--------|------|
| 200 | 成功 |
| 400 | 请求参数错误 |
| 401 | 认证失败 |
| 403 | 权限不足 |
| 404 | 资源不存在 |
| 500 | 服务器内部错误 |

## 9. 消息处理流程

### 9.1 客户端发送消息流程
1. 构造JSON数据
2. 调用`buildPacket()`打包消息
3. 通过TCP Socket发送

### 9.2 服务器处理消息流程
1. 接收TCP数据
2. 调用`drainPackets()`拆包
3. MessageRouter根据消息类型分发
4. 对应的ProtocolHandler处理业务逻辑
5. 返回响应或广播消息

### 9.3 消息验证规则
- 所有消息必须包含`room_id`字段（认证消息除外）
- 时间戳`ts`字段用于消息排序和去重
- 二进制数据大小限制：视频帧≤1MB，音频帧≤64KB
- 文本消息长度限制：≤1000字符

## 10. 安全考虑

### 10.1 认证机制
- 用户必须先登录才能发送业务消息
- 会话超时机制：30分钟无活动自动断开
- 密码传输使用加密（建议HTTPS）

### 10.2 权限控制
- 工单创建者拥有最高权限
- 专家用户拥有设备控制权限
- 普通用户只能发送文本和设备数据

### 10.3 数据验证
- 服务器端验证所有输入参数
- 防止SQL注入和XSS攻击
- 限制消息频率防止DoS攻击

## 11. 性能优化

### 11.1 消息压缩
- JSON使用紧凑格式
- 视频帧支持JPEG压缩
- 音频帧支持PCM压缩

### 11.2 连接管理
- 支持长连接复用
- 心跳机制保持连接活跃
- 连接池管理提高并发性能

### 11.3 消息队列
- 异步处理大量消息
- 消息优先级队列
- 失败重试机制
