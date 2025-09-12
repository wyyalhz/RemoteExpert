#ifndef USER_MESSAGE_HANDLER_H
#define USER_MESSAGE_HANDLER_H

#include <QObject>
#include <QJsonObject>
#include "../../../logging/log_manager.h"

// 前向声明
class NetworkClient;

// 用户管理消息处理器 - 负责处理认证相关的消息
class UserMessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit UserMessageHandler(QObject *parent = nullptr);
    ~UserMessageHandler();

    // 设置网络客户端引用
    void setNetworkClient(NetworkClient* client);
    
    // 用户认证消息处理
    void handleLoginResponse(const QJsonObject& data);
    void handleRegisterResponse(const QJsonObject& data);
    void handleLogoutResponse(const QJsonObject& data);
    void handleHeartbeatResponse(const QJsonObject& data);

private:
    // 辅助方法
    bool validateMessageData(const QJsonObject& data, const QStringList& requiredFields);
    QString extractErrorMessage(const QJsonObject& data);

private:
    NetworkClient* networkClient_;
};

#endif // USER_MESSAGE_HANDLER_H
