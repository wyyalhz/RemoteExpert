#include "other_message_handler.h"
#include "../client/network_client.h"
#include "../../../logging/log_manager.h"
#include <QJsonDocument>

OtherMessageHandler::OtherMessageHandler(QObject *parent)
    : QObject(parent)
    , networkClient_(nullptr)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "其他消息处理器初始化完成");
}

OtherMessageHandler::~OtherMessageHandler()
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "其他消息处理器已销毁");
}

void OtherMessageHandler::setNetworkClient(NetworkClient* client)
{
    networkClient_ = client;
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "网络客户端引用已设置");
}

// 聊天消息处理
void OtherMessageHandler::handleTextMessage(const QJsonObject& data)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理文本消息");
    
    if (!validateMessageData(data, {"roomId", "text", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "文本消息数据格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString text = data["text"].toString();
    qint64 timestamp = data["timestamp"].toVariant().toLongLong();
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到文本消息 [%1]: %2").arg(roomId).arg(text));
}

void OtherMessageHandler::handleDeviceDataMessage(const QJsonObject& data)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理设备数据消息");
    
    if (!validateMessageData(data, {"roomId", "deviceType", "data", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "设备数据消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString deviceType = data["deviceType"].toString();
    QJsonObject deviceData = data["data"].toObject();
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到设备数据 [%1]: 类型=%2").arg(roomId).arg(deviceType));
}

void OtherMessageHandler::handleFileTransferMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理文件传输消息");
    
    if (!validateMessageData(data, {"roomId", "fileName", "fileSize", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "文件传输消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString fileName = data["fileName"].toString();
    qint64 fileSize = data["fileSize"].toVariant().toLongLong();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到文件传输 [%1]: %2 (%3 字节)").arg(roomId).arg(fileName).arg(fileSize));
}

void OtherMessageHandler::handleScreenshotMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理截图消息");
    
    if (!validateMessageData(data, {"roomId", "imageId", "width", "height", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "截图消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString imageId = data["imageId"].toString();
    int width = data["width"].toInt();
    int height = data["height"].toInt();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到截图 [%1]: %2 (%3x%4)").arg(roomId).arg(imageId).arg(width).arg(height));
}

// 音视频消息处理
void OtherMessageHandler::handleVideoFrameMessage(const QJsonObject& data, const QByteArray& binary)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理视频帧消息");
    
    if (!validateMessageData(data, {"roomId", "frameId", "width", "height", "fps", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "视频帧消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString frameId = data["frameId"].toString();
    int width = data["width"].toInt();
    int height = data["height"].toInt();
    int fps = data["fps"].toInt();
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到视频帧 [%1]: %2 (%3x%4, %5fps, %6字节)")
                    .arg(roomId).arg(frameId).arg(width).arg(height).arg(fps).arg(binary.size()));
}

void OtherMessageHandler::handleAudioFrameMessage(const QJsonObject& data, const QByteArray& binary)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理音频帧消息");
    
    if (!validateMessageData(data, {"roomId", "frameId", "sampleRate", "channels", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "音频帧消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString frameId = data["frameId"].toString();
    int sampleRate = data["sampleRate"].toInt();
    int channels = data["channels"].toInt();
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到音频帧 [%1]: %2 (%3Hz, %4通道, %5字节)")
                    .arg(roomId).arg(frameId).arg(sampleRate).arg(channels).arg(binary.size()));
}

void OtherMessageHandler::handleVideoControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理视频控制消息");
    
    if (!validateMessageData(data, {"roomId", "controlType", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "视频控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString controlType = data["controlType"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到视频控制 [%1]: %2").arg(roomId).arg(controlType));
}

void OtherMessageHandler::handleAudioControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理音频控制消息");
    
    if (!validateMessageData(data, {"roomId", "controlType", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "音频控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString controlType = data["controlType"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到音频控制 [%1]: %2").arg(roomId).arg(controlType));
}

// 控制消息处理
void OtherMessageHandler::handleControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理通用控制消息");
    
    if (!validateMessageData(data, {"roomId", "controlType", "target", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "通用控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString controlType = data["controlType"].toString();
    QString target = data["target"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到通用控制 [%1]: %2 -> %3").arg(roomId).arg(controlType).arg(target));
}

void OtherMessageHandler::handleDeviceControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理设备控制消息");
    
    if (!validateMessageData(data, {"roomId", "deviceId", "command", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "设备控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString deviceId = data["deviceId"].toString();
    QString command = data["command"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到设备控制 [%1]: 设备=%2, 命令=%3").arg(roomId).arg(deviceId).arg(command));
}

void OtherMessageHandler::handleSystemControlMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理系统控制消息");
    
    if (!validateMessageData(data, {"roomId", "command", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "系统控制消息格式无效");
        return;
    }
    
    QString roomId = data["roomId"].toString();
    QString command = data["command"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到系统控制 [%1]: %2").arg(roomId).arg(command));
}

// 系统消息处理
void OtherMessageHandler::handleServerEventMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理服务器事件消息");
    
    if (!validateMessageData(data, {"eventType", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "服务器事件消息格式无效");
        return;
    }
    
    QString eventType = data["eventType"].toString();
    QString roomId = data["roomId"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到服务器事件: %1 [%2]").arg(eventType).arg(roomId));
}

void OtherMessageHandler::handleErrorMessage(const QJsonObject& data)
{
    LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理错误消息");
    
    if (!validateMessageData(data, {"code", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "错误消息格式无效");
        return;
    }
    
    int code = data["code"].toInt();
    QString message = data["message"].toString();
    
    LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("服务器错误 [%1]: %2").arg(code).arg(message));
}

void OtherMessageHandler::handleNotificationMessage(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "处理通知消息");
    
    if (!validateMessageData(data, {"title", "message", "timestamp"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", "通知消息格式无效");
        return;
    }
    
    QString title = data["title"].toString();
    QString message = data["message"].toString();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                    QString("收到通知: %1 - %2").arg(title).arg(message));
}

bool OtherMessageHandler::validateMessageData(const QJsonObject& data, const QStringList& requiredFields)
{
    for (const QString& field : requiredFields) {
        if (!data.contains(field)) {
            LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "OtherMessageHandler", 
                           QString("缺少必需字段: %1").arg(field));
            return false;
        }
    }
    return true;
}

QString OtherMessageHandler::extractErrorMessage(const QJsonObject& data)
{
    if (data.contains("message")) {
        return data["message"].toString();
    }
    if (data.contains("error")) {
        return data["error"].toString();
    }
    return "未知错误";
}
