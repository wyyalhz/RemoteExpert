#include "network_logger.h"
#include "../../../common/protocol/protocol.h"

// 基本日志方法实现
void NetworkLogger::error(const QString &operation, const QString &message)
{
    LOG_ERROR(LogModule::NETWORK, LogLayer::NETWORK, operation, message);
}

void NetworkLogger::warning(const QString &operation, const QString &message)
{
    LOG_WARNING(LogModule::NETWORK, LogLayer::NETWORK, operation, message);
}

void NetworkLogger::info(const QString &operation, const QString &message)
{
    LOG_INFO(LogModule::NETWORK, LogLayer::NETWORK, operation, message);
}

void NetworkLogger::debug(const QString &operation, const QString &message)
{
    LOG_DEBUG(LogModule::NETWORK, LogLayer::NETWORK, operation, message);
}

// 连接相关日志实现
void NetworkLogger::connectionEstablished(const QString &clientInfo)
{
    QString message = QString("Connection established: %1").arg(formatClientInfo(clientInfo));
    LOG_INFO(LogModule::NETWORK, LogLayer::NETWORK, "Connection Established", message);
}

void NetworkLogger::connectionClosed(const QString &clientInfo, const QString &reason)
{
    QString message = QString("Connection closed: %1").arg(formatClientInfo(clientInfo));
    if (!reason.isEmpty()) {
        message += QString(" (Reason: %1)").arg(reason);
    }
    LOG_INFO(LogModule::NETWORK, LogLayer::NETWORK, "Connection Closed", message);
}

void NetworkLogger::connectionFailed(const QString &clientInfo, const QString &error)
{
    QString message = QString("Connection failed: %1 - %2").arg(formatClientInfo(clientInfo)).arg(error);
    LOG_ERROR(LogModule::NETWORK, LogLayer::NETWORK, "Connection Failed", message);
}

void NetworkLogger::connectionError(const QString &clientInfo, const QString &error)
{
    QString message = QString("Connection error: %1 - %2").arg(formatClientInfo(clientInfo)).arg(error);
    LOG_WARNING(LogModule::NETWORK, LogLayer::NETWORK, "Connection Error", message);
}

// 消息处理日志实现
void NetworkLogger::messageReceived(const QString &clientInfo, quint16 msgType, int dataSize)
{
    QString message = QString("Message received from %1: Type=%2, Size=%3 bytes")
                     .arg(formatClientInfo(clientInfo))
                     .arg(formatMessageType(msgType))
                     .arg(dataSize);
    LOG_DEBUG(LogModule::NETWORK, LogLayer::NETWORK, "Message Received", message);
}

void NetworkLogger::messageSent(const QString &clientInfo, quint16 msgType, int dataSize)
{
    QString message = QString("Message sent to %1: Type=%2, Size=%3 bytes")
                     .arg(formatClientInfo(clientInfo))
                     .arg(formatMessageType(msgType))
                     .arg(dataSize);
    LOG_DEBUG(LogModule::NETWORK, LogLayer::NETWORK, "Message Sent", message);
}

void NetworkLogger::messageRouting(const QString &clientInfo, quint16 msgType, const QString &handler)
{
    QString message = QString("Message routed from %1: Type=%2 -> Handler=%3")
                     .arg(formatClientInfo(clientInfo))
                     .arg(formatMessageType(msgType))
                     .arg(handler);
    LOG_DEBUG(LogModule::NETWORK, LogLayer::NETWORK, "Message Routing", message);
}

void NetworkLogger::messageUnhandled(const QString &clientInfo, quint16 msgType)
{
    QString message = QString("Unhandled message from %1: Type=%2")
                     .arg(formatClientInfo(clientInfo))
                     .arg(formatMessageType(msgType));
    LOG_WARNING(LogModule::NETWORK, LogLayer::NETWORK, "Message Unhandled", message);
}

// 房间管理日志实现
void NetworkLogger::userJoinedRoom(const QString &username, const QString &roomId)
{
    QString message = QString("User '%1' joined room '%2'").arg(username).arg(roomId);
    LOG_INFO(LogModule::NETWORK, LogLayer::NETWORK, "User Joined Room", message);
}

void NetworkLogger::userLeftRoom(const QString &username, const QString &roomId)
{
    QString message = QString("User '%1' left room '%2'").arg(username).arg(roomId);
    LOG_INFO(LogModule::NETWORK, LogLayer::NETWORK, "User Left Room", message);
}

void NetworkLogger::roomBroadcast(const QString &roomId, int memberCount, int dataSize)
{
    QString message = QString("Broadcast to room '%1': %2 members, %3 bytes")
                     .arg(roomId).arg(memberCount).arg(dataSize);
    LOG_DEBUG(LogModule::NETWORK, LogLayer::NETWORK, "Room Broadcast", message);
}

// 协议处理日志实现
void NetworkLogger::protocolError(const QString &clientInfo, const QString &operation, const QString &error)
{
    QString message = QString("Protocol error from %1 in %2: %3")
                     .arg(formatClientInfo(clientInfo))
                     .arg(operation)
                     .arg(error);
    LOG_ERROR(LogModule::NETWORK, LogLayer::NETWORK, "Protocol Error", message);
}

void NetworkLogger::authenticationSuccess(const QString &clientInfo, const QString &username)
{
    QString message = QString("Authentication successful for %1: User '%2'")
                     .arg(formatClientInfo(clientInfo))
                     .arg(username);
    LOG_INFO(LogModule::NETWORK, LogLayer::NETWORK, "Authentication Success", message);
}

void NetworkLogger::authenticationFailed(const QString &clientInfo, const QString &reason)
{
    QString message = QString("Authentication failed for %1: %2")
                     .arg(formatClientInfo(clientInfo))
                     .arg(reason);
    LOG_WARNING(LogModule::NETWORK, LogLayer::NETWORK, "Authentication Failed", message);
}

void NetworkLogger::authorizationFailed(const QString &clientInfo, const QString &operation, const QString &reason)
{
    QString message = QString("Authorization failed for %1 in %2: %3")
                     .arg(formatClientInfo(clientInfo))
                     .arg(operation)
                     .arg(reason);
    LOG_WARNING(LogModule::NETWORK, LogLayer::NETWORK, "Authorization Failed", message);
}

// 服务器状态日志实现
void NetworkLogger::serverStarted(const QHostAddress &address, quint16 port)
{
    QString message = QString("Server started on %1:%2")
                     .arg(address.toString()).arg(port);
    LOG_INFO(LogModule::NETWORK, LogLayer::NETWORK, "Server Started", message);
}

void NetworkLogger::serverStopped()
{
    LOG_INFO(LogModule::NETWORK, LogLayer::NETWORK, "Server Stopped", "Server stopped successfully");
}

void NetworkLogger::serverError(const QString &error)
{
    QString message = QString("Server error: %1").arg(error);
    LOG_ERROR(LogModule::NETWORK, LogLayer::NETWORK, "Server Error", message);
}

// 私有辅助方法实现
QString NetworkLogger::formatClientInfo(const QString &clientInfo)
{
    return clientInfo;
}

QString NetworkLogger::formatClientInfo(QTcpSocket* socket)
{
    if (!socket) {
        return "Unknown Client";
    }
    return QString("%1:%2")
           .arg(socket->peerAddress().toString())
           .arg(socket->peerPort());
}

QString NetworkLogger::formatMessageType(quint16 msgType)
{
    switch (msgType) {
        case MSG_LOGIN: return "LOGIN";
        case MSG_REGISTER: return "REGISTER";
        case MSG_LOGOUT: return "LOGOUT";
        case MSG_HEARTBEAT: return "HEARTBEAT";
        case MSG_CREATE_WORKORDER: return "CREATE_WORKORDER";
        case MSG_JOIN_WORKORDER: return "JOIN_WORKORDER";
        case MSG_LEAVE_WORKORDER: return "LEAVE_WORKORDER";
        case MSG_UPDATE_WORKORDER: return "UPDATE_WORKORDER";
        case MSG_LIST_WORKORDERS: return "LIST_WORKORDERS";
        case MSG_TEXT: return "TEXT";
        case MSG_DEVICE_DATA: return "DEVICE_DATA";
        case MSG_FILE_TRANSFER: return "FILE_TRANSFER";
        case MSG_SCREENSHOT: return "SCREENSHOT";
        case MSG_VIDEO_FRAME: return "VIDEO_FRAME";
        case MSG_AUDIO_FRAME: return "AUDIO_FRAME";
        case MSG_VIDEO_CONTROL: return "VIDEO_CONTROL";
        case MSG_AUDIO_CONTROL: return "AUDIO_CONTROL";
        case MSG_CONTROL: return "CONTROL";
        case MSG_DEVICE_CONTROL: return "DEVICE_CONTROL";
        case MSG_SYSTEM_CONTROL: return "SYSTEM_CONTROL";
        case MSG_SERVER_EVENT: return "SERVER_EVENT";
        case MSG_ERROR: return "ERROR";
        case MSG_NOTIFICATION: return "NOTIFICATION";
        default: return QString("UNKNOWN(%1)").arg(msgType);
    }
}

QString NetworkLogger::formatRoomInfo(const QString &roomId, int memberCount)
{
    return QString("Room '%1' (%2 members)").arg(roomId).arg(memberCount);
}
