#include "message_builder.h"
#include <QDateTime>
#include <QRegularExpression>

// MessageBuilder 实现
QJsonObject MessageBuilder::buildLoginMessage(const QString& username, 
                                            const QString& password, 
                                            int userType,
                                            int userId)
{
    QJsonObject obj{
        {"username", username},
        {"password", password},
        {"user_type", userType},
        {"timestamp", QDateTime::currentMSecsSinceEpoch()}
    };
    
    // 如果提供了有效的用户ID，则添加到响应中
    if (userId > 0) {
        obj["id"] = userId;
    }
    
    return obj;
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
                                                      const QString& expertUsername,
                                                      const QJsonObject& deviceInfo)
{
    QJsonObject obj{
        {"title", title},
        {"description", description},
        {"priority", priority},
        {"category", category},
        {"expert_username", expertUsername},
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
