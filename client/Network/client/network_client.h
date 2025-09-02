#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <QObject>
#include <QHostAddress>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include "../connection/connection_manager.h"
#include "../protocol/handlers/message_handler.h"
#include "../../Logger/log_manager.h"

// 网络客户端主类 - 整合所有网络组件
class NetworkClient : public QObject
{
    Q_OBJECT
public:
    explicit NetworkClient(QObject *parent = nullptr);
    ~NetworkClient();

    // 连接管理
    bool connectToServer(const QString& host, quint16 port);
    bool connectToServer(const QHostAddress& address, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;
    
    // 消息发送
    bool sendMessage(quint16 type, const QJsonObject& data, const QByteArray& binary = QByteArray());
    bool sendLoginRequest(const QString& username, const QString& password, int userType);
    bool sendRegisterRequest(const QString& username, const QString& password, 
                           const QString& email, const QString& phone, int userType);
    bool sendLogoutRequest();
    bool sendCreateTicketRequest(const QString& title, const QString& description, 
                               const QString& priority, const QString& category, 
                               const QString& expertUsername, const QJsonObject& deviceInfo = QJsonObject());
    bool sendJoinTicketRequest(const QString& ticketId, const QString& role);
    bool sendLeaveTicketRequest(const QString& ticketId);
    bool sendGetTicketListRequest(const QString& status = QString(), int limit = -1, int offset = 0);
    bool sendGetTicketDetailRequest(const QString& ticketId, int userId, int userType);
    bool sendUpdateTicketRequest(const QJsonObject& ticketData);
    bool sendUpdateStatusRequest(int ticketId, const QString& newStatus);
    bool sendAssignTicketRequest(int ticketId, int assigneeId);
    bool sendDeleteTicketRequest(int ticketId);
    
    // 状态查询
    QString getLastError() const;
    QString getConnectionStatus() const;
    
    // 心跳管理
    void startHeartbeat(int interval = 30000); // 30秒
    void stopHeartbeat();

signals:
    // 连接状态信号
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    
    // 认证响应信号
    void loginResponse(const QJsonObject& response);
    void registerResponse(const QJsonObject& response);
    void logoutResponse(const QJsonObject& response);
    
    // 工单响应信号
    void createTicketResponse(const QJsonObject& response);
    void joinTicketResponse(const QJsonObject& response);
    void leaveTicketResponse(const QJsonObject& response);
    void getTicketListResponse(const QJsonObject& response);
    void getTicketDetailResponse(const QJsonObject& response);
    void updateTicketResponse(const QJsonObject& response);
    void updateStatusResponse(const QJsonObject& response);
    void assignTicketResponse(const QJsonObject& response);
    void deleteTicketResponse(const QJsonObject& response);
    
    // 系统消息信号
    void serverEvent(const QJsonObject& event);
    void errorMessage(const QJsonObject& error);
    void notification(const QJsonObject& notification);

private slots:
    // 连接状态处理
    void onConnected();
    void onDisconnected();
    void onConnectionError(const QString& error);
    
    // 消息处理
    void onMessageReceived(quint16 type, const QJsonObject& data, const QByteArray& binary);
    
    // 心跳处理
    void onHeartbeatTimeout();

private:
    // 私有辅助方法
    void setupConnections();
    void setupMessageHandlers();
    void logMessage(quint16 type, const QJsonObject& data, bool isOutgoing);
    int convertPriorityToInt(const QString& priority);

private:
    ConnectionManager* connectionManager_;
    MessageHandler* messageHandler_;
    QTimer* heartbeatTimer_;
    
    QString lastError_;
    bool isConnected_;
    QString connectionStatus_;
    

};

#endif // NETWORK_CLIENT_H
