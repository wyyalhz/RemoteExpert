#ifndef NETWORK_LOGGER_H
#define NETWORK_LOGGER_H

#include "../../logging/managers/log_manager.h"
#include <QTcpSocket>
#include <QHostAddress>

// 网络层日志助手类
class NetworkLogger
{
public:
    // 基本日志方法
    static void error(const QString &operation, const QString &message);
    static void warning(const QString &operation, const QString &message);
    static void info(const QString &operation, const QString &message);
    static void debug(const QString &operation, const QString &message);
    
    // 连接相关日志
    static void connectionEstablished(const QString &clientInfo);
    static void connectionClosed(const QString &clientInfo, const QString &reason = QString());
    static void connectionFailed(const QString &clientInfo, const QString &error);
    static void connectionError(const QString &clientInfo, const QString &error);
    
    // 消息处理日志
    static void messageReceived(const QString &clientInfo, quint16 msgType, int dataSize);
    static void messageSent(const QString &clientInfo, quint16 msgType, int dataSize);
    static void messageRouting(const QString &clientInfo, quint16 msgType, const QString &handler);
    static void messageUnhandled(const QString &clientInfo, quint16 msgType);
    
    // 房间管理日志
    static void userJoinedRoom(const QString &username, const QString &roomId);
    static void userLeftRoom(const QString &username, const QString &roomId);
    static void roomBroadcast(const QString &roomId, int memberCount, int dataSize);
    
    // 协议处理日志
    static void protocolError(const QString &clientInfo, const QString &operation, const QString &error);
    static void authenticationSuccess(const QString &clientInfo, const QString &username);
    static void authenticationFailed(const QString &clientInfo, const QString &reason);
    static void authorizationFailed(const QString &clientInfo, const QString &operation, const QString &reason);
    
    // 服务器状态日志
    static void serverStarted(const QHostAddress &address, quint16 port);
    static void serverStopped();
    static void serverError(const QString &error);

private:
    // 格式化客户端信息
    static QString formatClientInfo(const QString &clientInfo);
    static QString formatClientInfo(QTcpSocket* socket);
    
    // 格式化消息类型
    static QString formatMessageType(quint16 msgType);
    
    // 格式化房间信息
    static QString formatRoomInfo(const QString &roomId, int memberCount);
};

#endif // NETWORK_LOGGER_H
