#include "message_validator.h"
#include <QRegularExpression>

// MessageValidator 实现
bool MessageValidator::validateLoginMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "username", error)) return false;
    if (!validateRequiredField(data, "password", error)) return false;
    if (!validateRequiredField(data, "user_type", error)) return false;
    
    QString username = data["username"].toString();
    QString password = data["password"].toString();
    int userType = data["user_type"].toInt();
    
    if (!validateStringLength(username, ValidationRules::MAX_USERNAME_LENGTH, "username", error)) return false;
    if (!validateStringLength(password, ValidationRules::MAX_PASSWORD_LENGTH, "password", error)) return false;
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
    
    if (!validateStringLength(username, ValidationRules::MAX_USERNAME_LENGTH, "username", error)) return false;
    if (!validateStringLength(password, ValidationRules::MAX_PASSWORD_LENGTH, "password", error)) return false;
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
    if (!validateRequiredField(data, "expert_username", error)) return false;
    
    QString title = data["title"].toString();
    QString description = data["description"].toString();
    int priority = data["priority"].toInt();
    QString expertUsername = data["expert_username"].toString();
    
    if (!validateStringLength(title, ValidationRules::MAX_TITLE_LENGTH, "title", error)) return false;
    if (!validateStringLength(description, ValidationRules::MAX_DESCRIPTION_LENGTH, "description", error)) return false;
    if (!validateIntegerRange(priority, 1, 3, "priority", error)) return false;
    if (!validateStringLength(expertUsername, ValidationRules::MAX_USERNAME_LENGTH, "expert_username", error)) return false;
    
    return true;
}

bool MessageValidator::validateJoinWorkOrderMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "roomId", error)) return false;
    if (!validateRequiredField(data, "role", error)) return false;
    
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
    if (!validateRequiredField(data, "status", error)) return false;
    
    return true;
}

bool MessageValidator::validateListWorkOrdersMessage(const QJsonObject& data, QString& error)
{
    // 可选字段，不需要验证
    return true;
}

bool MessageValidator::validateTextMessage(const QJsonObject& data, QString& error)
{
    if (!validateRequiredField(data, "roomId", error)) return false;
    if (!validateRequiredField(data, "text", error)) return false;
    if (!validateRequiredField(data, "timestamp", error)) return false;
    
    QString text = data["text"].toString();
    if (!validateStringLength(text, ValidationRules::MAX_TEXT_LENGTH, "text", error)) return false;
    
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
    
    if (!validateIntegerRange(width, 1, 7680, "width", error)) return false; // 最大8K
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
    if (!validateRequiredField(data, "params", error)) return false;
    if (!validateRequiredField(data, "timestamp", error)) return false;
    
    return true;
}

// 通用验证方法
bool MessageValidator::validateRequiredField(const QJsonObject& data, 
                                           const QString& fieldName, 
                                           QString& error)
{
    if (!data.contains(fieldName) || data[fieldName].isNull() || data[fieldName].isUndefined()) {
        error = QString("Missing required field: %1").arg(fieldName);
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
        error = QString("Field %1 exceeds maximum length of %2 characters").arg(fieldName).arg(maxLength);
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
        error = QString("Field %1 must be between %2 and %3").arg(fieldName).arg(minValue).arg(maxValue);
        return false;
    }
    return true;
}

bool MessageValidator::validatePacket(const Packet& packet)
{
    // 验证数据包的基本结构
    if (packet.type == 0) {
        return false; // 无效的消息类型
    }
    
    // 验证JSON数据不为空
    if (packet.json.isEmpty()) {
        return false; // JSON数据不能为空
    }
    
    // 验证二进制数据大小（如果有的话）
    if (packet.bin.size() > ValidationRules::MAX_FILE_SIZE) {
        return false; // 二进制数据过大
    }
    
    return true;
}
