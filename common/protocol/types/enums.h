#pragma once
// ===============================================
// common/protocol/types/enums.h
// 协议消息类型和业务枚举定义
// ===============================================

#include <QString>

// 消息类型枚举
enum MsgType : quint16 {
    // 认证类消息 (1-9)
    MSG_REGISTER         = 1,   // 用户注册
    MSG_LOGIN            = 2,   // 用户登录
    MSG_LOGOUT           = 3,   // 用户登出
    MSG_HEARTBEAT        = 4,   // 心跳包
    
    // 工单类消息 (10-19)
    MSG_CREATE_WORKORDER = 10,  // 创建工单
    MSG_JOIN_WORKORDER   = 11,  // 加入工单
    MSG_LEAVE_WORKORDER  = 12,  // 离开工单
    MSG_UPDATE_WORKORDER = 13,  // 更新工单状态
    MSG_LIST_WORKORDERS  = 14,  // 获取工单列表
    
    // 聊天类消息 (20-29)
    MSG_TEXT             = 20,  // 文本消息
    MSG_DEVICE_DATA      = 21,  // 设备数据
    MSG_FILE_TRANSFER    = 22,  // 文件传输
    MSG_SCREENSHOT      = 23,  // 截图
    
    // 音视频类消息 (30-49)
    MSG_VIDEO_FRAME      = 30,  // 视频帧
    MSG_AUDIO_FRAME      = 31,  // 音频帧
    MSG_VIDEO_CONTROL    = 32,  // 视频控制
    MSG_AUDIO_CONTROL    = 33,  // 音频控制
    
    // 控制类消息 (50-59)
    MSG_CONTROL          = 50,  // 通用控制指令
    MSG_DEVICE_CONTROL   = 51,  // 设备控制
    MSG_SYSTEM_CONTROL   = 52,  // 系统控制
    
    // 系统类消息 (90-99)
    MSG_SERVER_EVENT     = 90,  // 服务器事件
    MSG_ERROR            = 91,  // 错误消息
    MSG_NOTIFICATION     = 92   // 通知消息
};

// 用户类型枚举
enum UserType : int {
    USER_TYPE_NORMAL     = 0,   // 普通用户
    USER_TYPE_EXPERT     = 1    // 专家用户
};

// 工单优先级枚举
enum WorkOrderPriority : int {
    PRIORITY_LOW         = 1,   // 低优先级
    PRIORITY_MEDIUM      = 2,   // 中优先级
    PRIORITY_HIGH        = 3    // 高优先级
};

// 工单状态枚举
enum WorkOrderStatus : int {
    STATUS_CREATED       = 1,  // 已创建
    STATUS_ASSIGNED      = 2,  // 已分配
    STATUS_IN_PROGRESS   = 3,  // 进行中
    STATUS_RESOLVED      = 4,  // 已解决
    STATUS_CLOSED        = 5   // 已关闭
};

// 控制指令类型枚举
enum ControlType : int {
    CONTROL_START_VIDEO  = 1,  // 开始视频
    CONTROL_STOP_VIDEO   = 2,  // 停止视频
    CONTROL_START_AUDIO  = 3,  // 开始音频
    CONTROL_STOP_AUDIO   = 4,  // 停止音频
    CONTROL_SCREENSHOT   = 5,  // 截图
    CONTROL_DEVICE_CTRL  = 6   // 设备控制
};

// 事件类型枚举
enum EventType : int {
    EVENT_USER_JOINED    = 1,  // 用户加入
    EVENT_USER_LEFT      = 2,  // 用户离开
    EVENT_ROOM_CREATED   = 3,  // 房间创建
    EVENT_ROOM_CLOSED    = 4,  // 房间关闭
    EVENT_WORKORDER_UPDATED = 5 // 工单更新
};
