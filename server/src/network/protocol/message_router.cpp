#include "message_router.h"
#include "protocol_handler.h"
#include "../logging/network_logger.h"

MessageRouter::MessageRouter(QObject *parent)
    : QObject(parent)
{
}

MessageRouter::~MessageRouter()
{
    // 注意：这里不删除handlers_，因为它们是外部管理的
    handlers_.clear();
}

void MessageRouter::registerHandler(quint16 msgType, ProtocolHandler* handler)
{
    if (!handler) {
        NetworkLogger::warning("Message Router", 
                              QString("Attempted to register null handler for message type %1").arg(msgType));
        return;
    }
    
    handlers_[msgType] = handler;
    
    NetworkLogger::info("Message Router", 
                        QString("Registered handler for message type %1").arg(msgType));
}

void MessageRouter::unregisterHandler(quint16 msgType)
{
    if (handlers_.remove(msgType) > 0) {
        NetworkLogger::info("Message Router", 
                            QString("Unregistered handler for message type %1").arg(msgType));
    }
}

void MessageRouter::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    if (!socket) {
        NetworkLogger::error("Message Router", "Received message with null socket");
        return;
    }
    
    ProtocolHandler* handler = getHandler(packet.type);
    if (handler) {
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::messageRouting(clientInfo, packet.type, handler->metaObject()->className());
        
        handler->handleMessage(socket, packet);
    } else {
        logUnhandledMessage(packet.type, socket);
    }
}

ProtocolHandler* MessageRouter::getHandler(quint16 msgType) const
{
    return handlers_.value(msgType, nullptr);
}

QList<quint16> MessageRouter::getRegisteredMessageTypes() const
{
    return handlers_.keys();
}

void MessageRouter::logUnhandledMessage(quint16 msgType, QTcpSocket* socket)
{
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::messageUnhandled(clientInfo, msgType);
}
