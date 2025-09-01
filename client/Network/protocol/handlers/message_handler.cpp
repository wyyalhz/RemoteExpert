#include "message_handler.h"
#include "../client/network_client.h"
#include "../../../Logger/log_manager.h"
#include "../../../../common/protocol/protocol.h"
#include <QJsonDocument>

MessageHandler::MessageHandler(QObject *parent)
    : QObject(parent)
    , networkClient_(nullptr)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "消息处理器初始化完成");
}

MessageHandler::~MessageHandler()
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "消息处理器已销毁");
}

void MessageHandler::setNetworkClient(NetworkClient* client)
{
    networkClient_ = client;
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "网络客户端引用已设置");
}

void MessageHandler::handleMessage(quint16 type, const QJsonObject& data, const QByteArray& binary)
{
    logMessageHandling(type, "开始处理");
    
    // 根据消息类型分发到相应的处理方法
    switch (type) {
        case MSG_LOGIN:
            handleLoginResponse(data);
            break;
        case MSG_REGISTER:
            handleRegisterResponse(data);
            break;
        case MSG_LOGOUT:
            handleLogoutResponse(data);
            break;
        case MSG_HEARTBEAT:
            handleHeartbeatResponse(data);
            break;
        case MSG_CREATE_WORKORDER:
            handleCreateWorkOrderResponse(data);
            break;
        case MSG_JOIN_WORKORDER:
            handleJoinWorkOrderResponse(data);
            break;
        case MSG_LEAVE_WORKORDER:
            handleLeaveWorkOrderResponse(data);
            break;
        case MSG_UPDATE_WORKORDER:
            handleUpdateWorkOrderResponse(data);
            break;
        case MSG_LIST_WORKORDERS:
            handleListWorkOrdersResponse(data);
            break;
        case MSG_TEXT:
            handleTextMessage(data);
            break;
        case MSG_DEVICE_DATA:
            handleDeviceDataMessage(data);
            break;
        case MSG_FILE_TRANSFER:
            handleFileTransferMessage(data);
            break;
        case MSG_SCREENSHOT:
            handleScreenshotMessage(data);
            break;
        case MSG_VIDEO_FRAME:
            handleVideoFrameMessage(data, binary);
            break;
        case MSG_AUDIO_FRAME:
            handleAudioFrameMessage(data, binary);
            break;
        case MSG_VIDEO_CONTROL:
            handleVideoControlMessage(data);
            break;
        case MSG_AUDIO_CONTROL:
            handleAudioControlMessage(data);
            break;
        case MSG_CONTROL:
            handleControlMessage(data);
            break;
        case MSG_DEVICE_CONTROL:
            handleDeviceControlMessage(data);
            break;
        case MSG_SYSTEM_CONTROL:
            handleSystemControlMessage(data);
            break;
        case MSG_SERVER_EVENT:
            handleServerEventMessage(data);
            break;
        case MSG_ERROR:
            handleErrorMessage(data);
            break;
        case MSG_NOTIFICATION:
            handleNotificationMessage(data);
            break;
        default:
            LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                                             QString("未知消息类型: %1").arg(type));
            break;
    }
    
    logMessageHandling(type, "处理完成");
}

void MessageHandler::handleLoginResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理登录响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "登录响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "登录成功");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", QString("登录失败: %1").arg(message));
    }
}

void MessageHandler::handleRegisterResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理注册响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "注册响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "注册成功");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", QString("注册失败: %1").arg(message));
    }
}

void MessageHandler::handleLogoutResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理登出响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "登出响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "登出成功");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", QString("登出失败: %1").arg(message));
    }
}

void MessageHandler::handleHeartbeatResponse(const QJsonObject& data)
{
    // 心跳响应通常不需要特殊处理，只是确认连接正常
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "收到心跳响应");
}

void MessageHandler::handleCreateWorkOrderResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理创建工单响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "创建工单响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        QString ticketId = data["ticketId"].toString();
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                        QString("工单创建成功，ID: %1").arg(ticketId));
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                        QString("工单创建失败: %1").arg(message));
    }
}

void MessageHandler::handleJoinWorkOrderResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理加入工单响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "加入工单响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        QString roomId = data["roomId"].toString();
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                        QString("成功加入工单房间: %1").arg(roomId));
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                        QString("加入工单失败: %1").arg(message));
    }
}

void MessageHandler::handleLeaveWorkOrderResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理离开工单响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "离开工单响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "成功离开工单");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                        QString("离开工单失败: %1").arg(message));
    }
}

void MessageHandler::handleUpdateWorkOrderResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理更新工单响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "更新工单响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "工单更新成功");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                        QString("工单更新失败: %1").arg(message));
    }
}

void MessageHandler::handleListWorkOrdersResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理获取工单列表响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "获取工单列表响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        QJsonArray workOrders = data["workOrders"].toArray();
        int totalCount = data["totalCount"].toInt();
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                        QString("成功获取工单列表，共 %1 个工单").arg(totalCount));
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                        QString("获取工单列表失败: %1").arg(message));
    }
}

void MessageHandler::handleTextMessage(const QJsonObject& data)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理文本消息");
    
    if (!validateMessageData(data, {"roomId", "text", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "文本消息数据格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString text = data["text"].toString();
    qint64 timestamp = data["timestamp"].toVariant().toLongLong();
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到文本消息 [%1]: %2").arg(roomId).arg(text));
}

void MessageHandler::handleDeviceDataMessage(const QJsonObject& data)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理设备数据消息");
    
    if (!validateMessageData(data, {"roomId", "deviceType", "data", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "设备数据消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString deviceType = data["deviceType"].toString();
    QJsonObject deviceData = data["data"].toObject();
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到设备数据 [%1]: 类型=%2").arg(roomId).arg(deviceType));
}

void MessageHandler::handleFileTransferMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理文件传输消息");
    
    if (!validateMessageData(data, {"roomId", "fileName", "fileSize", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "文件传输消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString fileName = data["fileName"].toString();
    qint64 fileSize = data["fileSize"].toVariant().toLongLong();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到文件传输 [%1]: %2 (%3 字节)").arg(roomId).arg(fileName).arg(fileSize));
}

void MessageHandler::handleScreenshotMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理截图消息");
    
    if (!validateMessageData(data, {"roomId", "imageId", "width", "height", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "截图消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString imageId = data["imageId"].toString();
    int width = data["width"].toInt();
    int height = data["height"].toInt();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到截图 [%1]: %2 (%3x%4)").arg(roomId).arg(imageId).arg(width).arg(height));
}

void MessageHandler::handleVideoFrameMessage(const QJsonObject& data, const QByteArray& binary)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理视频帧消息");
    
    if (!validateMessageData(data, {"roomId", "frameId", "width", "height", "fps", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "视频帧消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString frameId = data["frameId"].toString();
    int width = data["width"].toInt();
    int height = data["height"].toInt();
    int fps = data["fps"].toInt();
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到视频帧 [%1]: %2 (%3x%4, %5fps, %6字节)")
                    .arg(roomId).arg(frameId).arg(width).arg(height).arg(fps).arg(binary.size()));
}

void MessageHandler::handleAudioFrameMessage(const QJsonObject& data, const QByteArray& binary)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理音频帧消息");
    
    if (!validateMessageData(data, {"roomId", "frameId", "sampleRate", "channels", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "音频帧消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString frameId = data["frameId"].toString();
    int sampleRate = data["sampleRate"].toInt();
    int channels = data["channels"].toInt();
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到音频帧 [%1]: %2 (%3Hz, %4通道, %5字节)")
                    .arg(roomId).arg(frameId).arg(sampleRate).arg(channels).arg(binary.size()));
}

void MessageHandler::handleVideoControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理视频控制消息");
    
    if (!validateMessageData(data, {"roomId", "controlType", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "视频控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString controlType = data["controlType"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到视频控制 [%1]: %2").arg(roomId).arg(controlType));
}

void MessageHandler::handleAudioControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理音频控制消息");
    
    if (!validateMessageData(data, {"roomId", "controlType", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "音频控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString controlType = data["controlType"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到音频控制 [%1]: %2").arg(roomId).arg(controlType));
}

void MessageHandler::handleControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理通用控制消息");
    
    if (!validateMessageData(data, {"roomId", "controlType", "target", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "通用控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString controlType = data["controlType"].toString();
    QString target = data["target"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到通用控制 [%1]: %2 -> %3").arg(roomId).arg(controlType).arg(target));
}

void MessageHandler::handleDeviceControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理设备控制消息");
    
    if (!validateMessageData(data, {"roomId", "deviceId", "command", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "设备控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString deviceId = data["deviceId"].toString();
    QString command = data["command"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到设备控制 [%1]: 设备=%2, 命令=%3").arg(roomId).arg(deviceId).arg(command));
}

void MessageHandler::handleSystemControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理系统控制消息");
    
    if (!validateMessageData(data, {"roomId", "command", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "系统控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString command = data["command"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到系统控制 [%1]: %2").arg(roomId).arg(command));
}

void MessageHandler::handleServerEventMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理服务器事件消息");
    
    if (!validateMessageData(data, {"eventType", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "服务器事件消息格式无效");
        return;
    }
    
    QString eventType = data["eventType"].toString();
    QString roomId = data["roomId"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到服务器事件: %1 [%2]").arg(eventType).arg(roomId));
}

void MessageHandler::handleErrorMessage(const QJsonObject& data)
{
    LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理错误消息");
    
    if (!validateMessageData(data, {"code", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "错误消息格式无效");
        return;
    }
    
    int code = data["code"].toInt();
    QString message = data["message"].toString();
    
    LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("服务器错误 [%1]: %2").arg(code).arg(message));
}

void MessageHandler::handleNotificationMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "处理通知消息");
    
    if (!validateMessageData(data, {"title", "message", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "通知消息格式无效");
        return;
    }
    
    QString title = data["title"].toString();
    QString message = data["message"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("收到通知: %1 - %2").arg(title).arg(message));
}

void MessageHandler::logMessageHandling(quint16 type, const QString& action)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("消息处理 [%1]: %2").arg(type).arg(action));
}

bool MessageHandler::validateMessageData(const QJsonObject& data, const QStringList& requiredFields)
{
    for (const QString& field : requiredFields) {
        if (!data.contains(field)) {
            LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                           QString("缺少必需字段: %1").arg(field));
            return false;
        }
    }
    return true;
}

QString MessageHandler::extractErrorMessage(const QJsonObject& data)
{
    if (data.contains("message")) {
        return data["message"].toString();
    }
    if (data.contains("error")) {
        return data["error"].toString();
    }
    return "未知错误";
}
