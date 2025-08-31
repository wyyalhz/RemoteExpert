#ifndef MESSAGE_ROUTER_H
#define MESSAGE_ROUTER_H

#include <QObject>
#include <QHash>
#include <QTcpSocket>
#include "../../../common/protocol/protocol.h"

class ProtocolHandler;

// 消息路由器 - 负责根据消息类型分发到对应的处理器
class MessageRouter : public QObject
{
    Q_OBJECT
public:
    explicit MessageRouter(QObject *parent = nullptr);
    ~MessageRouter();

    // 注册消息处理器
    void registerHandler(quint16 msgType, ProtocolHandler* handler);
    void unregisterHandler(quint16 msgType);
    
    // 处理消息
    void handleMessage(QTcpSocket* socket, const Packet& packet);
    
    // 获取处理器
    ProtocolHandler* getHandler(quint16 msgType) const;
    
    // 获取已注册的消息类型
    QList<quint16> getRegisteredMessageTypes() const;

private:
    QHash<quint16, ProtocolHandler*> handlers_;
    
    void logUnhandledMessage(quint16 msgType, QTcpSocket* socket);
};

#endif // MESSAGE_ROUTER_H
