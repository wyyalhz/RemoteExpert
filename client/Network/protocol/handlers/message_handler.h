#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "../../../Logger/log_manager.h"

// 前向声明
class NetworkClient;

// 消息处理器 - 负责处理各种类型的消息
class MessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit MessageHandler(QObject *parent = nullptr);
    ~MessageHandler();

    // 设置网络客户端引用
    void setNetworkClient(NetworkClient* client);
    
    // 消息处理入口
    void handleMessage(quint16 type, const QJsonObject& data, const QByteArray& binary);

private:
    // 认证消息处理
    void handleLoginResponse(const QJsonObject& data);
    void handleRegisterResponse(const QJsonObject& data);
    void handleLogoutResponse(const QJsonObject& data);
    void handleHeartbeatResponse(const QJsonObject& data);
    
    // 工单消息处理
    void handleCreateWorkOrderResponse(const QJsonObject& data);
    void handleJoinWorkOrderResponse(const QJsonObject& data);
    void handleLeaveWorkOrderResponse(const QJsonObject& data);
    void handleUpdateWorkOrderResponse(const QJsonObject& data);
    void handleListWorkOrdersResponse(const QJsonObject& data);
    
    // 聊天消息处理
    void handleTextMessage(const QJsonObject& data);
    void handleDeviceDataMessage(const QJsonObject& data);
    void handleFileTransferMessage(const QJsonObject& data);
    void handleScreenshotMessage(const QJsonObject& data);
    
    // 音视频消息处理
    void handleVideoFrameMessage(const QJsonObject& data, const QByteArray& binary);
    void handleAudioFrameMessage(const QJsonObject& data, const QByteArray& binary);
    void handleVideoControlMessage(const QJsonObject& data);
    void handleAudioControlMessage(const QJsonObject& data);
    
    // 控制消息处理
    void handleControlMessage(const QJsonObject& data);
    void handleDeviceControlMessage(const QJsonObject& data);
    void handleSystemControlMessage(const QJsonObject& data);
    
    // 系统消息处理
    void handleServerEventMessage(const QJsonObject& data);
    void handleErrorMessage(const QJsonObject& data);
    void handleNotificationMessage(const QJsonObject& data);
    
    // 辅助方法
    void logMessageHandling(quint16 type, const QString& action);
    bool validateMessageData(const QJsonObject& data, const QStringList& requiredFields);
    QString extractErrorMessage(const QJsonObject& data);

private:
    NetworkClient* networkClient_;
};

#endif // MESSAGE_HANDLER_H
