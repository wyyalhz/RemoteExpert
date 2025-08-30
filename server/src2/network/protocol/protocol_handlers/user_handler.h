#ifndef USER_HANDLER_H
#define USER_HANDLER_H

#include "../protocol_handler.h"
#include "../../../business/services/user_service.h"

// 用户协议处理器 - 处理用户相关的消息
class UserHandler : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit UserHandler(UserService* userService, QObject *parent = nullptr);
    ~UserHandler();

    // 实现基类的消息处理方法
    void handleMessage(QTcpSocket* socket, const Packet& packet) override;

private:
    UserService* userService_;
    
    // 处理具体的用户消息
    void handleLogin(QTcpSocket* socket, const QJsonObject& data);
    void handleRegister(QTcpSocket* socket, const QJsonObject& data);
    void handleLogout(QTcpSocket* socket, const QJsonObject& data);
    
    // 辅助方法
    void updateClientAuthentication(QTcpSocket* socket, const QString& username, bool authenticated);
};

#endif // USER_HANDLER_H
