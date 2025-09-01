#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <QObject>
#include <QHostAddress>
#include "server/tcp_server.h"
#include "connection_manager.h"
#include "protocol/message_router.h"
#include "protocol/protocol_handlers/user_handler.h"
#include "protocol/protocol_handlers/workorder_handler.h"
#include "protocol/protocol_handlers/chat_handler.h"
#include "../../business/services/user_service.h"
#include "../../business/services/workorder_service.h"
#include "../../business/services/session_service.h"

// 网络服务器主类 - 整合所有网络组件
class NetworkServer : public QObject
{
    Q_OBJECT
public:
    explicit NetworkServer(QObject *parent = nullptr);
    ~NetworkServer();

    // 初始化网络服务器
    bool initialize(UserService* userService, WorkOrderService* workOrderService);
    
    // 启动服务器
    bool start(quint16 port);
    bool start(const QHostAddress &address, quint16 port);
    
    // 停止服务器
    void stop();
    
    // 获取服务器状态
    bool isRunning() const;
    QString getLastError() const;
    
    // 获取统计信息
    int getConnectionCount() const;
    int getRoomCount() const;

private:
    TCPServer* tcpServer_;
    ConnectionManager* connectionManager_;
    MessageRouter* messageRouter_;
    
    // 协议处理器
    UserHandler* userHandler_;
    WorkOrderHandler* workOrderHandler_;
    ChatHandler* chatHandler_;
    
    // 业务服务
    UserService* userService_;
    WorkOrderService* workOrderService_;
    SessionService* sessionService_;
    
    // 注册消息处理器
    void registerMessageHandlers();
    
    // 设置组件间的连接
    void setupConnections();

private slots:
    void onServerStarted();
    void onServerStopped();
    void onConnectionCountChanged();
};

#endif // NETWORK_SERVER_H
