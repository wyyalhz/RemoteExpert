#include "protocol_types.h"
#include <QDateTime>
#include <QRegularExpression>

// MessageBuilder 实现
QJsonObject MessageBuilder::buildLoginMessage(const QString& username, 
                                            const QString& password, 
                                            int userType)
{
    return QJsonObject{
        {"username", username},
        {"password", password},
        {"user_type", userType},
        {"timestamp", QDateTime::currentMSecsSinceEpoch()}
    };
}

QJsonObject MessageBuilder::buildRegisterMessage(const QString& username,
                                               const QString& password,
                                               const QString& email,
                                               const QString& phone,
                                               int userType)
{
    QJsonObject obj{
        {"username", username},
        {"password", password},
        {"user_type", userType},
        {"timestamp", QDateTime::currentMSecsSinceEpoch()}
    };
    
    if (!email.isEmpty()) obj["email"] = email;
    if (!phone.isEmpty()) obj["phone"] = phone;
    
    return obj;
}

QJsonObject MessageBuilder::buildCreateWorkOrderMessage(const QString& title,
                                                      const QString& description,
                                                      int priority,
                                                      const QString& category,
                                                      const QJsonObject& deviceInfo)
{
    QJsonObject obj{
        {"title", title},
        {"description", description},
        {"priority", priority},
        {"category", category},
        {"timestamp", QDateTime::currentMSecsSinceEpoch()}
    };
    
    if (!deviceInfo.isEmpty()) obj["device_info"] = deviceInfo;
    
    return obj;
}

QJsonObject MessageBuilder::buildJoinWorkOrderMessage(const QString& workorderId,
                                                    const QString& role)
{
    return QJsonObject{
        {"roomId", workorderId},
        {"role", role},
        {"timestamp", QDateTime::currentMSecsSinceEpoch()}
    };
}

QJsonObject MessageBuilder::buildTextMessage(const QString& roomId,
                                           const QString& text,
                                           qint64 timestamp,
                                           const QString& messageId)
{
    QJsonObject obj{
        {"roomId", roomId},
        {"text", text},
        {"timestamp", timestamp}
    };
    
    if (!messageId.isEmpty()) obj["messageId"] = messageId;
    
    return obj;
}

QJsonObject MessageBuilder::buildDeviceDataMessage(const QString& roomId,
                                                 const QString& deviceType,
                                                 const QJsonObject& data,
                                                 qint64 timestamp)
{
    return QJsonObject{
        {"roomId", roomId},
        {"deviceType", deviceType},
        {"data", data},
        {"timestamp", timestamp}
    };
}

QJsonObject MessageBuilder::buildVideoFrameMessage(const QString& roomId,
                                                 const QString& frameId,
                                                 int width,
                                                 int height,
                                                 int fps,
                                                 qint64 timestamp)
{
    return QJsonObject{
        {"roomId", roomId},
        {"frameId", frameId},
        {"width", width},
        {"height", height},
        {"fps", fps},
        {"timestamp", timestamp}
    };
}

QJsonObject MessageBuilder::buildAudioFrameMessage(const QString& roomId,
                                                 const QString& frameId,
                                                 int sampleRate,
                                                 int channels,
                                                 qint64 timestamp)
{
    return QJsonObject{
        {"roomId", roomId},
        {"frameId", frameId},
        {"sampleRate", sampleRate},
        {"channels", channels},
        {"timestamp", timestamp}
    };
}

QJsonObject MessageBuilder::buildControlMessage(const QString& roomId,
                                              const QString& controlType,
                                              const QString& target,
                                              const QJsonObject& params,
                                              qint64 timestamp)
{
    return QJsonObject{
        {"roomId", roomId},
        {"controlType", controlType},
        {"target", target},
        {"params", params},
        {"timestamp", timestamp}
    };
}

QJsonObject MessageBuilder::buildServerEventMessage(const QString& eventType,
                                                  const QString& roomId,
                                                  const QJsonObject& data,
                                                  qint64 timestamp)
{
    return QJsonObject{
        {"eventType", eventType},
        {"roomId", roomId},
        {"data", data},
        {"timestamp", timestamp}
    };
}

QJsonObject MessageBuilder::buildErrorMessage(int code,
                                            const QString& message,
                                            const QJsonObject& data)
{
    QJsonObject obj{
        {"code", code},
        {"message", message}
    };
    
    if (!data.isEmpty()) obj["data"] = data;
    
    return obj;
}

QJsonObject MessageBuilder::buildSuccessResponse(const QString& message,
                                               const QJsonObject& data)
{
    QJsonObject obj{
        {"code", 0},
        {"message", message}
    };
    
    if (!data.isEmpty()) {
        for (auto it = data.begin(); it != data.end(); ++it) {
            obj[it.key()] = it.value();
        }
    }
    
    return obj;
}

QJsonObject MessageBuilder::buildErrorResponse(int code,
                                             const QString& message,
                                             const QJsonObject& data)
{
    return buildErrorMessage(code, message, data);
}

QJsonObject MessageBuilder::buildWorkOrderCreatedResponse(const QString& ticketId,
                                                        const QString& title,
                                                        const QString& priority,
                                                        const QString& category)
{
    return QJsonObject{
        {"ticket_id", ticketId},
        {"title", title},
        {"priority", priority},
        {"category", category}
    };
}

QJsonObject MessageBuilder::buildWorkOrderJoinedResponse(const QString& roomId,
                                                       const QJsonObject& workOrderInfo)
{
    return QJsonObject{
        {"roomId", roomId},
        {"work_order_info", workOrderInfo}
    };
}

QJsonObject MessageBuilder::buildWorkOrderLeftResponse(const QString& roomId)
{
    return QJsonObject{
        {"roomId", roomId}
    };
}

QJsonObject MessageBuilder::buildWorkOrderUpdatedResponse(const QString& ticketId,
                                                        const QString& status)
{
    return QJsonObject{
        {"ticket_id", ticketId},
        {"status", status}
    };
}

QJsonObject MessageBuilder::buildWorkOrderListResponse(const QJsonArray& workOrders,
                                                     int totalCount)
{
    return QJsonObject{
        {"work_orders", workOrders},
        {"total_count", totalCount}
    };
}

QJsonObject MessageBuilder::buildHeartbeatResponse(qint64 timestamp)
{
    return QJsonObject{
        {"timestamp", timestamp}
    };
}

// MessageValidator 实现
bool MessageValidator::validateLoginMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "username", error)) return false;
    if (!validateRequiredField(data, "password", error)) return false;
    if (!validateRequiredField(data, "user_type", error)) return false;
    
    QString username = data["username"].toString();
    QString password = data["password"].toString();
    int userType = data["user_type"].toInt();
    
    if (!validateStringLength(username, MessageValidationRules::MAX_USERNAME_LENGTH, "username", error)) return false;
    if (!validateStringLength(password, MessageValidationRules::MAX_PASSWORD_LENGTH, "password", error)) return false;
    if (!validateIntegerRange(userType, 0, 1, "user_type", error)) return false;
    
    return true;
}

bool MessageValidator::validateRegisterMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "username", error)) return false;
    if (!validateRequiredField(data, "password", error)) return false;
    if (!validateRequiredField(data, "user_type", error)) return false;
    
    QString username = data["username"].toString();
    QString password = data["password"].toString();
    QString email = data["email"].toString();
    QString phone = data["phone"].toString();
    int userType = data["user_type"].toInt();
    
    if (!validateStringLength(username, MessageValidationRules::MAX_USERNAME_LENGTH, "username", error)) return false;
    if (!validateStringLength(password, MessageValidationRules::MAX_PASSWORD_LENGTH, "password", error)) return false;
    if (!validateIntegerRange(userType, 0, 1, "user_type", error)) return false;
    
    // 验证邮箱格式（如果提供）
    if (!email.isEmpty()) {
        QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        if (!emailRegex.match(email).hasMatch()) {
            error = "Invalid email format";
            return false;
        }
    }
    
    // 验证手机号格式（如果提供）
    if (!phone.isEmpty()) {
        QRegularExpression phoneRegex("^1[3-9]\\d{9}$");
        if (!phoneRegex.match(phone).hasMatch()) {
            error = "Invalid phone number format";
            return false;
        }
    }
    
    return true;
}

bool MessageValidator::validateCreateWorkOrderMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "title", error)) return false;
    if (!validateRequiredField(data, "description", error)) return false;
    if (!validateRequiredField(data, "priority", error)) return false;
    if (!validateRequiredField(data, "category", error)) return false;
    
    QString title = data["title"].toString();
    QString description = data["description"].toString();
    int priority = data["priority"].toInt();
    
    if (!validateStringLength(title, MessageValidationRules::MAX_TITLE_LENGTH, "title", error)) return false;
    if (!validateStringLength(description, MessageValidationRules::MAX_DESCRIPTION_LENGTH, "description", error)) return false;
    if (!validateIntegerRange(priority, 1, 3, "priority", error)) return false;
    
    return true;
}

bool MessageValidator::validateJoinWorkOrderMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "roomId", error)) return false;
    if (!validateRequiredField(data, "role", error)) return false;
    
    QString role = data["role"].toString();
    if (role != "expert" && role != "user") {
        error = "Invalid role value";
        return false;
    }
    
    return true;
}

bool MessageValidator::validateLeaveWorkOrderMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "roomId", error)) return false;
    return true;
}

bool MessageValidator::validateUpdateWorkOrderMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "ticket_id", error)) return false;
    
    // status和description是可选的，但如果提供则验证格式
    if (data.contains("status")) {
        QString status = data["status"].toString();
        if (status.isEmpty()) {
            error = "Status cannot be empty if provided";
            return false;
        }
    }
    
    if (data.contains("description")) {
        QString description = data["description"].toString();
        if (!validateStringLength(description, MessageValidationRules::MAX_DESCRIPTION_LENGTH, "description", error)) return false;
    }
    
    return true;
}

bool MessageValidator::validateListWorkOrdersMessage(const QJsonObject& data, QString& error)
{
    // 所有字段都是可选的，但如果提供则验证格式
    if (data.contains("status")) {
        QString status = data["status"].toString();
        if (status.isEmpty()) {
            error = "Status cannot be empty if provided";
            return false;
        }
    }
    
    if (data.contains("limit")) {
        int limit = data["limit"].toInt();
        if (!validateIntegerRange(limit, 1, 100, "limit", error)) return false;
    }
    
    if (data.contains("offset")) {
        int offset = data["offset"].toInt();
        if (!validateIntegerRange(offset, 0, 10000, "offset", error)) return false;
    }
    
    return true;
}

bool MessageValidator::validateTextMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "roomId", error)) return false;
    if (!validateRequiredField(data, "text", error)) return false;
    if (!validateRequiredField(data, "timestamp", error)) return false;
    
    QString text = data["text"].toString();
    if (!validateStringLength(text, MessageValidationRules::MAX_TEXT_LENGTH, "text", error)) return false;
    
    return true;
}

bool MessageValidator::validateDeviceDataMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "roomId", error)) return false;
    if (!validateRequiredField(data, "deviceType", error)) return false;
    if (!validateRequiredField(data, "data", error)) return false;
    if (!validateRequiredField(data, "timestamp", error)) return false;
    
    return true;
}

bool MessageValidator::validateVideoFrameMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "roomId", error)) return false;
    if (!validateRequiredField(data, "frameId", error)) return false;
    if (!validateRequiredField(data, "width", error)) return false;
    if (!validateRequiredField(data, "height", error)) return false;
    if (!validateRequiredField(data, "fps", error)) return false;
    if (!validateRequiredField(data, "timestamp", error)) return false;
    
    int width = data["width"].toInt();
    int height = data["height"].toInt();
    int fps = data["fps"].toInt();
    
    if (!validateIntegerRange(width, 1, 7680, "width", error)) return false;  // 最大8K
    if (!validateIntegerRange(height, 1, 4320, "height", error)) return false; // 最大8K
    if (!validateIntegerRange(fps, 1, 120, "fps", error)) return false;
    
    return true;
}

bool MessageValidator::validateAudioFrameMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "roomId", error)) return false;
    if (!validateRequiredField(data, "frameId", error)) return false;
    if (!validateRequiredField(data, "sampleRate", error)) return false;
    if (!validateRequiredField(data, "channels", error)) return false;
    if (!validateRequiredField(data, "timestamp", error)) return false;
    
    int sampleRate = data["sampleRate"].toInt();
    int channels = data["channels"].toInt();
    
    if (!validateIntegerRange(sampleRate, 8000, 192000, "sampleRate", error)) return false;
    if (!validateIntegerRange(channels, 1, 8, "channels", error)) return false;
    
    return true;
}

bool MessageValidator::validateControlMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "roomId", error)) return false;
    if (!validateRequiredField(data, "controlType", error)) return false;
    if (!validateRequiredField(data, "target", error)) return false;
    if (!validateRequiredField(data, "timestamp", error)) return false;
    
    return true;
}

bool MessageValidator::validateRequiredField(const QJsonObject& data, 
                                           const QString& fieldName, 
                                           QString& error)
{
    if (!data.contains(fieldName) || data[fieldName].isNull()) {
        error = QString("Required field '%1' is missing or null").arg(fieldName);
        return false;
    }
    return true;
}

bool MessageValidator::validateStringLength(const QString& value, 
                                          int maxLength, 
                                          const QString& fieldName, 
                                          QString& error)
{
    if (value.length() > maxLength) {
        error = QString("Field '%1' exceeds maximum length of %2 characters").arg(fieldName).arg(maxLength);
        return false;
    }
    return true;
}

bool MessageValidator::validateIntegerRange(int value, 
                                          int minValue, 
                                          int maxValue, 
                                          const QString& fieldName, 
                                          QString& error)
{
    if (value < minValue || value > maxValue) {
        error = QString("Field '%1' must be between %2 and %3").arg(fieldName).arg(minValue).arg(maxValue);
        return false;
    }
    return true;
}

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
    
    return true;
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
    
    return true;
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
    
    return true;
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
    
    return true;
}

bool MessageParser::parseLeaveWorkOrderMessage(const QJsonObject& data,
                                              QString& roomId)
{
    if (!data.contains("roomId")) {
        return false;
    }
    
    roomId = data["roomId"].toString();
    return true;
}

bool MessageParser::parseUpdateWorkOrderMessage(const QJsonObject& data,
                                               QString& ticketId,
                                               QString& status,
                                               QString& description)
{
    if (!data.contains("ticket_id")) {
        return false;
    }
    
    ticketId = data["ticket_id"].toString();
    status = data["status"].toString();
    description = data["description"].toString();
    
    return true;
}

bool MessageParser::parseListWorkOrdersMessage(const QJsonObject& data,
                                              QString& status,
                                              int& limit,
                                              int& offset)
{
    status = data["status"].toString();
    limit = data["limit"].toInt(50);
    offset = data["offset"].toInt(0);
    
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
    
    return true;
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
    
    return true;
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
    
    return true;
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
    
    return true;
}

bool MessageParser::parseControlMessage(const QJsonObject& data,
                                       QString& roomId,
                                       QString& controlType,
                                       QString& target,
                                       QJsonObject& params,
                                       qint64& timestamp)
{
    if (!data.contains("roomId") || !data.contains("controlType") || 
        !data.contains("target") || !data.contains("timestamp")) {
        return false;
    }
    
    roomId = data["roomId"].toString();
    controlType = data["controlType"].toString();
    target = data["target"].toString();
    params = data["params"].toObject();
    timestamp = data["timestamp"].toVariant().toLongLong();
    
    return true;
}

bool MessageParser::parseServerEventMessage(const QJsonObject& data,
                                           QString& eventType,
                                           QString& roomId,
                                           QJsonObject& eventData,
                                           qint64& timestamp)
{
    if (!data.contains("eventType") || !data.contains("timestamp")) {
        return false;
    }
    
    eventType = data["eventType"].toString();
    roomId = data["roomId"].toString();
    eventData = data["data"].toObject();
    timestamp = data["timestamp"].toVariant().toLongLong();
    
    return true;
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
    
    return true;
}
