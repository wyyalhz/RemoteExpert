#ifndef OTHER_MESSAGE_HANDLER_H
#define OTHER_MESSAGE_HANDLER_H

#include <QObject>
#include <QJsonObject>
#include <QByteArray>
#include "../../../Logger/log_manager.h"

// 前向声明
class NetworkClient;

// 其他管理消息处理器 - 负责处理聊天、音视频、控制等其他消息
class OtherMessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit OtherMessageHandler(QObject *parent = nullptr);
    ~OtherMessageHandler();

    // 设置网络客户端引用
    void setNetworkClient(NetworkClient* client);
    
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

private:
    // 辅助方法
    bool validateMessageData(const QJsonObject& data, const QStringList& requiredFields);
    QString extractErrorMessage(const QJsonObject& data);

private:
    NetworkClient* networkClient_;
};

#endif // OTHER_MESSAGE_HANDLER_H
