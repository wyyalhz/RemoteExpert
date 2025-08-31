#include "message_parser.h"

// MessageParser 实现
bool MessageParser::parseLoginMessage(const QJsonObject& data,
                                     QString& username,
                                     QString& password,
                                     int& userType)
{
    if (!data.contains("username") || !data.contains("password") || !data.contains("user_type")) {
        return false;
    }
    
    username = data["username"].toString();
    password = data["password"].toString();
    userType = data["user_type"].toInt();
    
    return !username.isEmpty() && !password.isEmpty();
}

bool MessageParser::parseRegisterMessage(const QJsonObject& data,
                                        QString& username,
                                        QString& password,
                                        QString& email,
                                        QString& phone,
                                        int& userType)
{
    if (!data.contains("username") || !data.contains("password") || !data.contains("user_type")) {
        return false;
    }
    
    username = data["username"].toString();
    password = data["password"].toString();
    email = data["email"].toString();
    phone = data["phone"].toString();
    userType = data["user_type"].toInt();
    
    return !username.isEmpty() && !password.isEmpty();
}

bool MessageParser::parseCreateWorkOrderMessage(const QJsonObject& data,
                                               QString& title,
                                               QString& description,
                                               int& priority,
                                               QString& category,
                                               QJsonObject& deviceInfo)
{
    if (!data.contains("title") || !data.contains("description") || 
        !data.contains("priority") || !data.contains("category")) {
        return false;
    }
    
    title = data["title"].toString();
    description = data["description"].toString();
    priority = data["priority"].toInt();
    category = data["category"].toString();
    deviceInfo = data["device_info"].toObject();
    
    return !title.isEmpty() && !description.isEmpty() && !category.isEmpty();
}

bool MessageParser::parseJoinWorkOrderMessage(const QJsonObject& data,
                                             QString& workorderId,
                                             QString& role)
{
    if (!data.contains("roomId") || !data.contains("role")) {
        return false;
    }
    
    workorderId = data["roomId"].toString();
    role = data["role"].toString();
    
    return !workorderId.isEmpty() && !role.isEmpty();
}

bool MessageParser::parseLeaveWorkOrderMessage(const QJsonObject& data,
                                              QString& roomId)
{
    if (!data.contains("roomId")) {
        return false;
    }
    
    roomId = data["roomId"].toString();
    return !roomId.isEmpty();
}

bool MessageParser::parseUpdateWorkOrderMessage(const QJsonObject& data,
                                               QString& ticketId,
                                               QString& status,
                                               QString& description)
{
    if (!data.contains("ticket_id") || !data.contains("status")) {
        return false;
    }
    
    ticketId = data["ticket_id"].toString();
    status = data["status"].toString();
    description = data["description"].toString();
    
    return !ticketId.isEmpty() && !status.isEmpty();
}

bool MessageParser::parseListWorkOrdersMessage(const QJsonObject& data,
                                              QString& status,
                                              int& limit,
                                              int& offset)
{
    status = data["status"].toString();
    limit = data["limit"].toInt(10);  // 默认10
    offset = data["offset"].toInt(0); // 默认0
    
    return true;
}

bool MessageParser::parseTextMessage(const QJsonObject& data,
                                    QString& roomId,
                                    QString& text,
                                    qint64& timestamp,
                                    QString& messageId)
{
    if (!data.contains("roomId") || !data.contains("text") || !data.contains("timestamp")) {
        return false;
    }
    
    roomId = data["roomId"].toString();
    text = data["text"].toString();
    timestamp = data["timestamp"].toVariant().toLongLong();
    messageId = data["messageId"].toString();
    
    return !roomId.isEmpty() && !text.isEmpty();
}

bool MessageParser::parseDeviceDataMessage(const QJsonObject& data,
                                          QString& roomId,
                                          QString& deviceType,
                                          QJsonObject& deviceData,
                                          qint64& timestamp)
{
    if (!data.contains("roomId") || !data.contains("deviceType") || 
        !data.contains("data") || !data.contains("timestamp")) {
        return false;
    }
    
    roomId = data["roomId"].toString();
    deviceType = data["deviceType"].toString();
    deviceData = data["data"].toObject();
    timestamp = data["timestamp"].toVariant().toLongLong();
    
    return !roomId.isEmpty() && !deviceType.isEmpty();
}

bool MessageParser::parseVideoFrameMessage(const QJsonObject& data,
                                          QString& roomId,
                                          QString& frameId,
                                          int& width,
                                          int& height,
                                          int& fps,
                                          qint64& timestamp)
{
    if (!data.contains("roomId") || !data.contains("frameId") || 
        !data.contains("width") || !data.contains("height") || 
        !data.contains("fps") || !data.contains("timestamp")) {
        return false;
    }
    
    roomId = data["roomId"].toString();
    frameId = data["frameId"].toString();
    width = data["width"].toInt();
    height = data["height"].toInt();
    fps = data["fps"].toInt();
    timestamp = data["timestamp"].toVariant().toLongLong();
    
    return !roomId.isEmpty() && !frameId.isEmpty() && width > 0 && height > 0 && fps > 0;
}

bool MessageParser::parseAudioFrameMessage(const QJsonObject& data,
                                          QString& roomId,
                                          QString& frameId,
                                          int& sampleRate,
                                          int& channels,
                                          qint64& timestamp)
{
    if (!data.contains("roomId") || !data.contains("frameId") || 
        !data.contains("sampleRate") || !data.contains("channels") || 
        !data.contains("timestamp")) {
        return false;
    }
    
    roomId = data["roomId"].toString();
    frameId = data["frameId"].toString();
    sampleRate = data["sampleRate"].toInt();
    channels = data["channels"].toInt();
    timestamp = data["timestamp"].toVariant().toLongLong();
    
    return !roomId.isEmpty() && !frameId.isEmpty() && sampleRate > 0 && channels > 0;
}

bool MessageParser::parseControlMessage(const QJsonObject& data,
                                       QString& roomId,
                                       QString& controlType,
                                       QString& target,
                                       QJsonObject& params,
                                       qint64& timestamp)
{
    if (!data.contains("roomId") || !data.contains("controlType") || 
        !data.contains("target") || !data.contains("params") || 
        !data.contains("timestamp")) {
        return false;
    }
    
    roomId = data["roomId"].toString();
    controlType = data["controlType"].toString();
    target = data["target"].toString();
    params = data["params"].toObject();
    timestamp = data["timestamp"].toVariant().toLongLong();
    
    return !roomId.isEmpty() && !controlType.isEmpty() && !target.isEmpty();
}

bool MessageParser::parseServerEventMessage(const QJsonObject& data,
                                           QString& eventType,
                                           QString& roomId,
                                           QJsonObject& eventData,
                                           qint64& timestamp)
{
    if (!data.contains("eventType") || !data.contains("roomId") || 
        !data.contains("data") || !data.contains("timestamp")) {
        return false;
    }
    
    eventType = data["eventType"].toString();
    roomId = data["roomId"].toString();
    eventData = data["data"].toObject();
    timestamp = data["timestamp"].toVariant().toLongLong();
    
    return !eventType.isEmpty() && !roomId.isEmpty();
}

bool MessageParser::parseErrorMessage(const QJsonObject& data,
                                     int& code,
                                     QString& message,
                                     QJsonObject& errorData)
{
    if (!data.contains("code") || !data.contains("message")) {
        return false;
    }
    
    code = data["code"].toInt();
    message = data["message"].toString();
    errorData = data["data"].toObject();
    
    return !message.isEmpty();
}
