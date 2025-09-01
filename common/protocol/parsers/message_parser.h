#pragma once
// ===============================================
// common/protocol/parsers/message_parser.h
// 消息解析工具类
// ===============================================

#include <QtCore>
#include <QtNetwork>
#include "../types/enums.h"

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
