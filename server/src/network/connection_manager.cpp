#include "connection_manager.h"
#include "protocol/message_router.h"
#include "logging/network_logger.h"
#include "../../../common/protocol/protocol.h"
#include "../business/services/session_service.h"
#include <QDateTime>

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent)
    , messageRouter_(nullptr)
    , sessionService_(nullptr)
{
}

ConnectionManager::~ConnectionManager()
{
    disconnectAll();
}

void ConnectionManager::addConnection(QTcpSocket* socket)
{
    if (!socket) return;
    
    QMutexLocker locker(&mutex_);
    
    auto* context = new ClientContext(socket);
    connections_[socket] = context;
    buffers_[socket] = QByteArray();
    
    setupSocketConnections(socket);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::connectionEstablished(clientInfo);
}

void ConnectionManager::removeConnection(QTcpSocket* socket)
{
    if (!socket) return;
    
    QMutexLocker locker(&mutex_);
    
    cleanupConnection(socket);
    socket->deleteLater();
}

void ConnectionManager::disconnectAll()
{
    QMutexLocker locker(&mutex_);
    
    for (auto it = connections_.begin(); it != connections_.end(); ++it) {
        QTcpSocket* socket = it.key();
        if (socket) {
            socket->disconnectFromHost();
        }
    }
    
    // 清理所有连接
    qDeleteAll(connections_);
    connections_.clear();
    userSockets_.clear();
    buffers_.clear();
    rooms_.clear();
    
    NetworkLogger::info("Connection Manager", "All connections disconnected");
}

QTcpSocket* ConnectionManager::getSocket(const QString& username)
{
    return userSockets_.value(username, nullptr);
}

ClientContext* ConnectionManager::getContext(QTcpSocket* socket)
{
    return connections_.value(socket, nullptr);
}

ClientContext* ConnectionManager::getContext(const QString& username)
{
    QTcpSocket* socket = getSocket(username);
    return socket ? getContext(socket) : nullptr;
}

void ConnectionManager::joinRoom(QTcpSocket* socket, const QString& roomId)
{
    if (!socket || roomId.isEmpty()) return;
    
    ClientContext* context = getContext(socket);
    if (!context) return;
    
    // 离开当前房间
    leaveRoom(socket);
    
    // 加入新房间
    context->currentRoom = roomId;
    rooms_[roomId].append(socket);
    
    NetworkLogger::userJoinedRoom(context->username, roomId);
}

void ConnectionManager::leaveRoom(QTcpSocket* socket)
{
    if (!socket) return;
    
    ClientContext* context = getContext(socket);
    if (!context || context->currentRoom.isEmpty()) return;
    
    QString roomId = context->currentRoom;
    rooms_[roomId].removeAll(socket);
    
    if (rooms_[roomId].isEmpty()) {
        rooms_.remove(roomId);
    }
    
    context->currentRoom.clear();
    
    NetworkLogger::userLeftRoom(context->username, roomId);
}

QString ConnectionManager::getCurrentRoom(QTcpSocket* socket)
{
    if(!socket)return QString();
    ClientContext* context = getContext(socket);
    if(!context)return QString();
    return context->currentRoom;
}

QList<QTcpSocket*> ConnectionManager::getRoomMembers(const QString& roomId)
{
    return rooms_.value(roomId, QList<QTcpSocket*>());
}

void ConnectionManager::broadcastToRoom(const QString& roomId, const QByteArray& data, QTcpSocket* except)
{
    QList<QTcpSocket*> members = getRoomMembers(roomId);
    
    for (QTcpSocket* socket : members) {
        if (socket != except) {
            sendToClient(socket, data);
        }
    }
    
    NetworkLogger::roomBroadcast(roomId, members.size(), data.size());
}

void ConnectionManager::sendToClient(QTcpSocket* socket, const QByteArray& data)
{
    if (!socket || data.isEmpty()) return;
    
    qint64 bytesWritten = socket->write(data);
    if (bytesWritten != data.size()) {
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::warning("Connection Manager", 
                              QString("Failed to send complete data: %1/%2 bytes")
                              .arg(bytesWritten).arg(data.size()));
    }
}

void ConnectionManager::sendToClient(const QString& username, const QByteArray& data)
{
    QTcpSocket* socket = getSocket(username);
    if (socket) {
        sendToClient(socket, data);
    }
}

void ConnectionManager::addUserSocket(const QString& username, QTcpSocket* socket)
{
    if (!username.isEmpty() && socket) {
        userSockets_[username] = socket;
    }
}

void ConnectionManager::removeUserSocket(const QString& username)
{
    if (!username.isEmpty()) {
        userSockets_.remove(username);
    }
}

int ConnectionManager::getConnectionCount() const
{
    return connections_.size();
}

int ConnectionManager::getRoomMemberCount(const QString& roomId) const
{
    return rooms_.value(roomId).size();
}

void ConnectionManager::setMessageRouter(MessageRouter* router)
{
    messageRouter_ = router;
}

void ConnectionManager::setupSocketConnections(QTcpSocket* socket)
{
    connect(socket, &QTcpSocket::readyRead, this, &ConnectionManager::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ConnectionManager::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &ConnectionManager::onError);
}

void ConnectionManager::cleanupConnection(QTcpSocket* socket)
{
    if (!socket) return;
    
    ClientContext* context = getContext(socket);
    if (context) {
        // 过期会话
        if (!context->sessionId.isEmpty()) {
            expireSession(socket);
        }
        
        // 离开房间
        leaveRoom(socket);
        
        // 从用户映射中移除
        if (!context->username.isEmpty()) {
            userSockets_.remove(context->username);
        }
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::connectionClosed(clientInfo, "User disconnected");
        
        delete context;
    }
    
    connections_.remove(socket);
    buffers_.remove(socket);
}

void ConnectionManager::updateLastActivity(QTcpSocket* socket)
{
    ClientContext* context = getContext(socket);
    if (context) {
        context->lastActivity = QDateTime::currentDateTime();
    }
}

void ConnectionManager::onReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    updateLastActivity(socket);
    
    // 更新会话活动
    updateSessionActivity(socket);
    
    QByteArray& buffer = buffers_[socket];
    QByteArray newData = socket->readAll();
    
    if (!newData.isEmpty()) {
        buffer.append(newData);
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::debug("Connection Manager", 
                            QString("Received %1 bytes from %2, buffer size: %3")
                            .arg(newData.size())
                            .arg(clientInfo)
                            .arg(buffer.size()));
        
        // 解析数据包
        QVector<Packet> packets;
        if (drainPackets(buffer, packets)) {
            for (const Packet& packet : packets) {
                if (messageRouter_) {
                    messageRouter_->handleMessage(socket, packet);
                } else {
                    NetworkLogger::error("Connection Manager", "Message router not set, cannot handle packet");
                }
            }
        }
    }
}

void ConnectionManager::onDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    cleanupConnection(socket);
}

void ConnectionManager::onError(QAbstractSocket::SocketError error)
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::connectionError(clientInfo, socket->errorString());
}

// 会话管理相关方法
void ConnectionManager::setSessionService(SessionService* sessionService)
{
    sessionService_ = sessionService;
}

bool ConnectionManager::createSessionForUser(QTcpSocket* socket, int userId, const QString& roomId)
{
    if (!sessionService_ || !socket) {
        return false;
    }
    
    QMutexLocker locker(&mutex_);
    
    ClientContext* context = getContext(socket);
    if (!context) {
        return false;
    }
    
    // 创建会话
    QString sessionId = sessionService_->createSession(userId, roomId);
    if (sessionId.isEmpty()) {
        NetworkLogger::error("Connection Manager", "Failed to create session for user");
        return false;
    }
    
    // 更新客户端上下文
    context->sessionId = sessionId;
    context->currentRoom = roomId;
    
    NetworkLogger::info("Connection Manager", 
                       QString("Session created for user %1 in room %2: %3")
                       .arg(userId).arg(roomId).arg(sessionId));
    return true;
}

bool ConnectionManager::updateSessionActivity(QTcpSocket* socket)
{
    if (!sessionService_ || !socket) {
        return false;
    }
    
    QMutexLocker locker(&mutex_);
    
    ClientContext* context = getContext(socket);
    if (!context || context->sessionId.isEmpty()) {
        return false;
    }
    
    bool success = sessionService_->updateSessionActivity(context->sessionId);
    if (success) {
        context->lastActivity = QDateTime::currentDateTime();
    }
    
    return success;
}

bool ConnectionManager::expireSession(QTcpSocket* socket)
{
    if (!sessionService_ || !socket) {
        return false;
    }
    
    QMutexLocker locker(&mutex_);
    
    ClientContext* context = getContext(socket);
    if (!context || context->sessionId.isEmpty()) {
        return false;
    }
    
    bool success = sessionService_->expireSession(context->sessionId);
    if (success) {
        context->sessionId.clear();
        context->currentRoom.clear();
    }
    
    return success;
}

bool ConnectionManager::isSessionValid(QTcpSocket* socket)
{
    if (!sessionService_ || !socket) {
        return false;
    }
    
    QMutexLocker locker(&mutex_);
    
    ClientContext* context = getContext(socket);
    if (!context || context->sessionId.isEmpty()) {
        return false;
    }
    
    return sessionService_->isSessionValid(context->sessionId);
}
