#pragma once
// ===============================================
// common/protocol_types.h
// 协议消息类型详细定义
// 包含所有消息类型的字段说明和验证规则
// ===============================================

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

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

// 枚举到字符串转换函数
namespace EnumConverter {
    QString workOrderStatusToString(WorkOrderStatus status);
    WorkOrderStatus stringToWorkOrderStatus(const QString& str);
    
    QString controlTypeToString(ControlType type);
    ControlType stringToControlType(const QString& str);
    
    QString eventTypeToString(EventType type);
    EventType stringToEventType(const QString& str);
}

// 消息字段验证规则
struct MessageValidationRules {
    // 通用字段
    static const int MAX_USERNAME_LENGTH = 50;
    static const int MAX_PASSWORD_LENGTH = 100;
    static const int MAX_TEXT_LENGTH = 1000;
    static const int MAX_TITLE_LENGTH = 200;
    static const int MAX_DESCRIPTION_LENGTH = 2000;
    
    // 二进制数据限制
    static const int MAX_VIDEO_FRAME_SIZE = 1024 * 1024;  // 1MB
    static const int MAX_AUDIO_FRAME_SIZE = 64 * 1024;    // 64KB
    static const int MAX_FILE_SIZE = 10 * 1024 * 1024;    // 10MB
    
    // 时间限制
    static const int HEARTBEAT_INTERVAL = 30;  // 30秒
    static const int SESSION_TIMEOUT = 1800;   // 30分钟
};

// 消息构建工具类
class MessageBuilder {
public:
    // 构建认证消息
    static QJsonObject buildLoginMessage(const QString& username, 
                                        const QString& password, 
                                        int userType);
    
    static QJsonObject buildRegisterMessage(const QString& username,
                                           const QString& password,
                                           const QString& email,
                                           const QString& phone,
                                           int userType);
    
    // 构建工单消息
    static QJsonObject buildCreateWorkOrderMessage(const QString& title,
                                                  const QString& description,
                                                  int priority,
                                                  const QString& category,
                                                  const QJsonObject& deviceInfo = QJsonObject());
    
    static QJsonObject buildJoinWorkOrderMessage(const QString& workorderId,
                                                const QString& role);
    
    // 构建聊天消息
    static QJsonObject buildTextMessage(const QString& roomId,
                                       const QString& text,
                                       qint64 timestamp,
                                       const QString& messageId = QString());
    
    static QJsonObject buildDeviceDataMessage(const QString& roomId,
                                             const QString& deviceType,
                                             const QJsonObject& data,
                                             qint64 timestamp);
    
    // 构建音视频消息
    static QJsonObject buildVideoFrameMessage(const QString& roomId,
                                             const QString& frameId,
                                             int width,
                                             int height,
                                             int fps,
                                             qint64 timestamp);
    
    static QJsonObject buildAudioFrameMessage(const QString& roomId,
                                             const QString& frameId,
                                             int sampleRate,
                                             int channels,
                                             qint64 timestamp);
    
    // 构建控制消息
    static QJsonObject buildControlMessage(const QString& roomId,
                                          const QString& controlType,
                                          const QString& target,
                                          const QJsonObject& params,
                                          qint64 timestamp);
    
    // 构建系统消息
    static QJsonObject buildServerEventMessage(const QString& eventType,
                                              const QString& roomId,
                                              const QJsonObject& data,
                                              qint64 timestamp);
    
    static QJsonObject buildErrorMessage(int code,
                                        const QString& message,
                                        const QJsonObject& data = QJsonObject());
    
    // 构建成功响应
    static QJsonObject buildSuccessResponse(const QString& message,
                                           const QJsonObject& data = QJsonObject());
    
    // 构建错误响应
    static QJsonObject buildErrorResponse(int code,
                                         const QString& message,
                                         const QJsonObject& data = QJsonObject());
    
    // 构建工单相关响应
    static QJsonObject buildWorkOrderCreatedResponse(const QString& ticketId,
                                                    const QString& title,
                                                    const QString& priority,
                                                    const QString& category);
    
    static QJsonObject buildWorkOrderJoinedResponse(const QString& roomId,
                                                   const QJsonObject& workOrderInfo);
    
    static QJsonObject buildWorkOrderLeftResponse(const QString& roomId);
    
    static QJsonObject buildWorkOrderUpdatedResponse(const QString& ticketId,
                                                    const QString& status);
    
    static QJsonObject buildWorkOrderListResponse(const QJsonArray& workOrders,
                                                 int totalCount);
    
    static QJsonObject buildHeartbeatResponse(qint64 timestamp);
};

// 消息验证工具类
class MessageValidator {
public:
    // 验证认证消息
    static bool validateLoginMessage(const QJsonObject& data, QString& error);
    static bool validateRegisterMessage(const QJsonObject& data, QString& error);
    
    // 验证工单消息
    static bool validateCreateWorkOrderMessage(const QJsonObject& data, QString& error);
    static bool validateJoinWorkOrderMessage(const QJsonObject& data, QString& error);
    static bool validateLeaveWorkOrderMessage(const QJsonObject& data, QString& error);
    static bool validateUpdateWorkOrderMessage(const QJsonObject& data, QString& error);
    static bool validateListWorkOrdersMessage(const QJsonObject& data, QString& error);
    
    // 验证聊天消息
    static bool validateTextMessage(const QJsonObject& data, QString& error);
    static bool validateDeviceDataMessage(const QJsonObject& data, QString& error);
    
    // 验证音视频消息
    static bool validateVideoFrameMessage(const QJsonObject& data, QString& error);
    static bool validateAudioFrameMessage(const QJsonObject& data, QString& error);
    
    // 验证控制消息
    static bool validateControlMessage(const QJsonObject& data, QString& error);
    
    // 通用验证方法
    static bool validateRequiredField(const QJsonObject& data, 
                                     const QString& fieldName, 
                                     QString& error);
    
    static bool validateStringLength(const QString& value, 
                                    int maxLength, 
                                    const QString& fieldName, 
                                    QString& error);
    
    static bool validateIntegerRange(int value, 
                                    int minValue, 
                                    int maxValue, 
                                    const QString& fieldName, 
                                    QString& error);
};

// 消息解析工具类
class MessageParser {
public:
    // 解析认证消息
    static bool parseLoginMessage(const QJsonObject& data,
                                 QString& username,
                                 QString& password,
                                 int& userType);
    
    static bool parseRegisterMessage(const QJsonObject& data,
                                    QString& username,
                                    QString& password,
                                    QString& email,
                                    QString& phone,
                                    int& userType);
    
    // 解析工单消息
    static bool parseCreateWorkOrderMessage(const QJsonObject& data,
                                           QString& title,
                                           QString& description,
                                           int& priority,
                                           QString& category,
                                           QJsonObject& deviceInfo);
    
    static bool parseJoinWorkOrderMessage(const QJsonObject& data,
                                         QString& workorderId,
                                         QString& role);
    
    static bool parseLeaveWorkOrderMessage(const QJsonObject& data,
                                          QString& roomId);
    
    static bool parseUpdateWorkOrderMessage(const QJsonObject& data,
                                           QString& ticketId,
                                           QString& status,
                                           QString& description);
    
    static bool parseListWorkOrdersMessage(const QJsonObject& data,
                                          QString& status,
                                          int& limit,
                                          int& offset);
    
    // 解析聊天消息
    static bool parseTextMessage(const QJsonObject& data,
                                QString& roomId,
                                QString& text,
                                qint64& timestamp,
                                QString& messageId);
    
    static bool parseDeviceDataMessage(const QJsonObject& data,
                                      QString& roomId,
                                      QString& deviceType,
                                      QJsonObject& deviceData,
                                      qint64& timestamp);
    
    // 解析音视频消息
    static bool parseVideoFrameMessage(const QJsonObject& data,
                                      QString& roomId,
                                      QString& frameId,
                                      int& width,
                                      int& height,
                                      int& fps,
                                      qint64& timestamp);
    
    static bool parseAudioFrameMessage(const QJsonObject& data,
                                      QString& roomId,
                                      QString& frameId,
                                      int& sampleRate,
                                      int& channels,
                                      qint64& timestamp);
    
    // 解析控制消息
    static bool parseControlMessage(const QJsonObject& data,
                                   QString& roomId,
                                   QString& controlType,
                                   QString& target,
                                   QJsonObject& params,
                                   qint64& timestamp);
    
    // 解析系统消息
    static bool parseServerEventMessage(const QJsonObject& data,
                                       QString& eventType,
                                       QString& roomId,
                                       QJsonObject& eventData,
                                       qint64& timestamp);
    
    static bool parseErrorMessage(const QJsonObject& data,
                                 int& code,
                                 QString& message,
                                 QJsonObject& errorData);
};

// 枚举转换函数实现
namespace EnumConverter {
    inline QString workOrderStatusToString(WorkOrderStatus status) {
        switch (status) {
            case STATUS_CREATED: return "created";
            case STATUS_ASSIGNED: return "assigned";
            case STATUS_IN_PROGRESS: return "in_progress";
            case STATUS_RESOLVED: return "resolved";
            case STATUS_CLOSED: return "closed";
            default: return "unknown";
        }
    }
    
    inline WorkOrderStatus stringToWorkOrderStatus(const QString& str) {
        if (str == "created") return STATUS_CREATED;
        if (str == "assigned") return STATUS_ASSIGNED;
        if (str == "in_progress") return STATUS_IN_PROGRESS;
        if (str == "resolved") return STATUS_RESOLVED;
        if (str == "closed") return STATUS_CLOSED;
        return STATUS_CREATED; // 默认值
    }
    
    inline QString controlTypeToString(ControlType type) {
        switch (type) {
            case CONTROL_START_VIDEO: return "start_video";
            case CONTROL_STOP_VIDEO: return "stop_video";
            case CONTROL_START_AUDIO: return "start_audio";
            case CONTROL_STOP_AUDIO: return "stop_audio";
            case CONTROL_SCREENSHOT: return "screenshot";
            case CONTROL_DEVICE_CTRL: return "device_control";
            default: return "unknown";
        }
    }
    
    inline ControlType stringToControlType(const QString& str) {
        if (str == "start_video") return CONTROL_START_VIDEO;
        if (str == "stop_video") return CONTROL_STOP_VIDEO;
        if (str == "start_audio") return CONTROL_START_AUDIO;
        if (str == "stop_audio") return CONTROL_STOP_AUDIO;
        if (str == "screenshot") return CONTROL_SCREENSHOT;
        if (str == "device_control") return CONTROL_DEVICE_CTRL;
        return CONTROL_START_VIDEO; // 默认值
    }
    
    inline QString eventTypeToString(EventType type) {
        switch (type) {
            case EVENT_USER_JOINED: return "user_joined";
            case EVENT_USER_LEFT: return "user_left";
            case EVENT_ROOM_CREATED: return "room_created";
            case EVENT_ROOM_CLOSED: return "room_closed";
            case EVENT_WORKORDER_UPDATED: return "workorder_updated";
            default: return "unknown";
        }
    }
    
    inline EventType stringToEventType(const QString& str) {
        if (str == "user_joined") return EVENT_USER_JOINED;
        if (str == "user_left") return EVENT_USER_LEFT;
        if (str == "room_created") return EVENT_ROOM_CREATED;
        if (str == "room_closed") return EVENT_ROOM_CLOSED;
        if (str == "workorder_updated") return EVENT_WORKORDER_UPDATED;
        return EVENT_USER_JOINED; // 默认值
    }
}
