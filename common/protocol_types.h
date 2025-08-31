#pragma once
// ===============================================
// common/protocol_types.h
// 协议消息类型详细定义
// 包含所有消息类型的字段说明和验证规则
// ===============================================

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

// 消息类型枚举（与protocol.h保持一致）
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
enum WorkOrderStatus : QString {
    STATUS_CREATED       = "created",      // 已创建
    STATUS_ASSIGNED      = "assigned",     // 已分配
    STATUS_IN_PROGRESS   = "in_progress",  // 进行中
    STATUS_RESOLVED      = "resolved",     // 已解决
    STATUS_CLOSED        = "closed"        // 已关闭
};

// 控制指令类型枚举
enum ControlType : QString {
    CONTROL_START_VIDEO  = "start_video",  // 开始视频
    CONTROL_STOP_VIDEO   = "stop_video",   // 停止视频
    CONTROL_START_AUDIO  = "start_audio",  // 开始音频
    CONTROL_STOP_AUDIO   = "stop_audio",   // 停止音频
    CONTROL_SCREENSHOT   = "screenshot",   // 截图
    CONTROL_DEVICE_CTRL  = "device_control" // 设备控制
};

// 事件类型枚举
enum EventType : QString {
    EVENT_USER_JOINED    = "user_joined",  // 用户加入
    EVENT_USER_LEFT      = "user_left",    // 用户离开
    EVENT_ROOM_CREATED   = "room_created", // 房间创建
    EVENT_ROOM_CLOSED    = "room_closed",  // 房间关闭
    EVENT_WORKORDER_UPDATED = "workorder_updated" // 工单更新
};

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
