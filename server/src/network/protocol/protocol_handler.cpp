#include "protocol_handler.h"
#include "../connection_manager.h"
#include "../logging/network_logger.h"
#include "../../../common/protocol/protocol.h"

ProtocolHandler::ProtocolHandler(QObject *parent)
    : QObject(parent)
    , connectionManager_(nullptr)
{
}

ProtocolHandler::~ProtocolHandler()
{
}

void ProtocolHandler::setConnectionManager(ConnectionManager* manager)
{
    connectionManager_ = manager;
}

ConnectionManager* ProtocolHandler::getConnectionManager() const
{
    return connectionManager_;
}

void ProtocolHandler::sendResponse(QTcpSocket* socket, const QJsonObject& response)
{
    sendResponse(socket, MSG_SERVER_EVENT, response);
}

void ProtocolHandler::sendResponse(QTcpSocket* socket, quint16 msgType, const QJsonObject& response)
{
    if (!socket || !connectionManager_) {
        NetworkLogger::error("Protocol Handler", "Cannot send response: socket or connection manager is null");
        return;
    }
    
    QByteArray packetData = buildPacket(msgType, response);
    connectionManager_->sendToClient(socket, packetData);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::messageSent(clientInfo, msgType, packetData.size());
}

void ProtocolHandler::sendErrorResponse(QTcpSocket* socket, int errorCode, const QString& message)
{
    // 使用MessageBuilder构建错误响应
    QJsonObject response = MessageBuilder::buildErrorResponse(errorCode, message);
    
    sendResponse(socket, response);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::protocolError(clientInfo, "Error Response", QString("%1 - %2").arg(errorCode).arg(message));
}

void ProtocolHandler::sendSuccessResponse(QTcpSocket* socket, const QString& message, const QJsonObject& data)
{
    // 使用MessageBuilder构建成功响应
    QJsonObject response = MessageBuilder::buildSuccessResponse(message, data);
    
    sendResponse(socket, response);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Protocol Handler", 
                         QString("Sent success response to %1: %2")
                         .arg(clientInfo)
                         .arg(message));
}

struct ClientContext* ProtocolHandler::getClientContext(QTcpSocket* socket)
{
    if (!connectionManager_) {
        return nullptr;
    }
    
    return connectionManager_->getContext(socket);
}

bool ProtocolHandler::checkAuthentication(QTcpSocket* socket)
{
    if (!connectionManager_) {
        return false;
    }
    
    ClientContext* context = connectionManager_->getContext(socket);
    if (!context || !context->isAuthenticated) {
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::authenticationFailed(clientInfo, "Authentication required");
        sendErrorResponse(socket, 403, "Authentication required. Please login first.");
        return false;
    }
    
    return true;
}

bool ProtocolHandler::checkRoomMembership(QTcpSocket* socket)
{
    if (!connectionManager_) {
        return false;
    }
    
    ClientContext* context = connectionManager_->getContext(socket);
    if (!context || context->currentRoom.isEmpty()) {
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::authorizationFailed(clientInfo, "Room Membership", "Please join a room first");
        sendErrorResponse(socket, 403, "Please join a room first.");
        return false;
    }
    
    return true;
}
