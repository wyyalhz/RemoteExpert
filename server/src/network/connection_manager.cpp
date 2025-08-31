#include "connection_manager.h"
#include "protocol/message_router.h"
#include "logging/network_logger.h"
#include "../../../common/protocol/protocol.h"
#include <QDateTime>

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent)
    , messageRouter_(nullptr)
{
}

ConnectionManager::~ConnectionManager()
{
    disconnectAll();
}

void ConnectionManager::addConnection(QTcpSocket* socket)
{
    if (!socket) return;
    
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
    
    cleanupConnection(socket);
    socket->deleteLater();
}

void ConnectionManager::disconnectAll()
{
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
