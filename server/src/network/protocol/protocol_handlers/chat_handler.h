#ifndef CHAT_HANDLER_H
#define CHAT_HANDLER_H

#include "../protocol_handler.h"

// 聊天协议处理器 - 处理聊天相关的消息
class ChatHandler : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit ChatHandler(QObject *parent = nullptr);
    ~ChatHandler();

    // 实现基类的消息处理方法
    void handleMessage(QTcpSocket* socket, const Packet& packet) override;

private:
    // 处理具体的聊天消息
    void handleTextMessage(QTcpSocket* socket, const Packet& packet);
    void handleDeviceData(QTcpSocket* socket, const Packet& packet);
    void handleVideoFrame(QTcpSocket* socket, const Packet& packet);
    void handleAudioFrame(QTcpSocket* socket, const Packet& packet);
    void handleControl(QTcpSocket* socket, const Packet& packet);
    
    // 辅助方法
    void broadcastToRoom(QTcpSocket* socket, const Packet& packet);
};

#endif // CHAT_HANDLER_H
