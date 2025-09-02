#pragma once
// ===============================================
// common/protocol/builders/message_builder.h
// 消息构建工具类
// ===============================================

#include <QtCore>
#include <QtNetwork>
#include "../types/enums.h"

// 消息构建工具类
class MessageBuilder {
public:
    // 构建认证消息
    static QJsonObject buildLoginMessage(const QString& username, 
                                        const QString& password, 
                                        int userType,
                                        int userId = -1);
    
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
                                                  const QString& expertUsername,
                                                  const QJsonObject& deviceInfo = QJsonObject());
    
    static QJsonObject buildJoinWorkOrderMessage(const QString& workorderId,
                                                const QString& role);
    
    static QJsonObject buildGetWorkOrderMessage(const QString& workorderId,
                                               int userId,
                                               int userType);
    
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
