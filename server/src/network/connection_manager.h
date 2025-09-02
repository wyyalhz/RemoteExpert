#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QHash>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QMutex>

class MessageRouter;

// 客户端上下文结构
struct ClientContext {
    QTcpSocket* socket = nullptr;
    QString username;
    int userId = -1;  // 添加用户ID
    QString currentRoom;
    QString sessionId;  // 添加会话ID
    bool isAuthenticated = false;
    QDateTime connectedAt;
    QDateTime lastActivity;
    
    ClientContext(QTcpSocket* sock) : socket(sock), connectedAt(QDateTime::currentDateTime()) {}
};

// 连接管理器 - 负责管理所有客户端连接
class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionManager(QObject *parent = nullptr);
    ~ConnectionManager();

    // 连接管理
    void addConnection(QTcpSocket* socket);
    void removeConnection(QTcpSocket* socket);
    void disconnectAll();
    
    // 连接查询
    QTcpSocket* getSocket(const QString& username);
    ClientContext* getContext(QTcpSocket* socket);
    ClientContext* getContext(const QString& username);
    
    // 房间管理
    void joinRoom(QTcpSocket* socket, const QString& roomId);
    void leaveRoom(QTcpSocket* socket);
    QString getCurrentRoom(QTcpSocket* socket);
    QList<QTcpSocket*> getRoomMembers(const QString& roomId);
    void broadcastToRoom(const QString& roomId, const QByteArray& data, QTcpSocket* except = nullptr);
    
    // 消息发送
    void sendToClient(QTcpSocket* socket, const QByteArray& data);
    void sendToClient(const QString& username, const QByteArray& data);
    
    // 用户映射管理
    void addUserSocket(const QString& username, QTcpSocket* socket);
    void removeUserSocket(const QString& username);
    
    // 统计信息
    int getConnectionCount() const;
    int getRoomMemberCount(const QString& roomId) const;
    
    // 设置消息路由器
    void setMessageRouter(MessageRouter* router);
    
    // 会话管理
    void setSessionService(class SessionService* sessionService);
    bool createSessionForUser(QTcpSocket* socket, int userId, const QString& roomId);
    bool updateSessionActivity(QTcpSocket* socket);
    bool expireSession(QTcpSocket* socket);
    bool isSessionValid(QTcpSocket* socket);

private slots:
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    QHash<QTcpSocket*, ClientContext*> connections_;
    QHash<QString, QTcpSocket*> userSockets_;
    QHash<QTcpSocket*, QByteArray> buffers_;
    QHash<QString, QList<QTcpSocket*>> rooms_;
    
    MessageRouter* messageRouter_;
    class SessionService* sessionService_;
    mutable QMutex mutex_;
    
    void setupSocketConnections(QTcpSocket* socket);
    void cleanupConnection(QTcpSocket* socket);
    void updateLastActivity(QTcpSocket* socket);
};

#endif // CONNECTION_MANAGER_H
