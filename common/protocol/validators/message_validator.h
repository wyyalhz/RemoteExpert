#pragma once
// ===============================================
// common/protocol/validators/message_validator.h
// 消息验证工具类
// ===============================================

#include <QtCore>
#include <QtNetwork>
#include "../types/enums.h"
#include "../types/validation_rules.h"

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
