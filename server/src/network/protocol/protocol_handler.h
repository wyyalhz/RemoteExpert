#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include "../../../common/protocol/protocol.h"

class ConnectionManager;

// 协议处理器基类 - 定义所有协议处理器的通用接口
class ProtocolHandler : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolHandler(QObject *parent = nullptr);
    virtual ~ProtocolHandler();

    // 处理消息的虚函数，子类必须实现
    virtual void handleMessage(QTcpSocket* socket, const Packet& packet) = 0;
    
    // 设置连接管理器
    void setConnectionManager(ConnectionManager* manager);
    
    // 获取连接管理器
    ConnectionManager* getConnectionManager() const;

protected:
    // 发送响应的辅助方法
    void sendResponse(QTcpSocket* socket, const QJsonObject& response);
    void sendResponse(QTcpSocket* socket, quint16 msgType, const QJsonObject& response);
    void sendErrorResponse(QTcpSocket* socket, int errorCode, const QString& message);
    void sendSuccessResponse(QTcpSocket* socket, const QString& message, const QJsonObject& data = QJsonObject());
    
    // 获取客户端上下文的辅助方法
    struct ClientContext* getClientContext(QTcpSocket* socket);
    
    // 认证检查的辅助方法
    bool checkAuthentication(QTcpSocket* socket);
    bool checkRoomMembership(QTcpSocket* socket);

private:
    ConnectionManager* connectionManager_;
};

#endif // PROTOCOL_HANDLER_H
