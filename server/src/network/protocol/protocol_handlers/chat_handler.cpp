#include "chat_handler.h"
#include "../connection_manager.h"
#include "../logging/network_logger.h"
#include "../../../common/protocol.h"

ChatHandler::ChatHandler(QObject *parent)
    : ProtocolHandler(parent)
{
}

ChatHandler::~ChatHandler()
{
}

void ChatHandler::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    // 检查认证状态
    if (!checkAuthentication(socket)) {
        return;
    }
    
    // 检查房间成员身份
    if (!checkRoomMembership(socket)) {
        return;
    }
    
    switch (packet.type) {
        case MSG_TEXT:
            handleTextMessage(socket, packet);
            break;
        case MSG_DEVICE_DATA:
            handleDeviceData(socket, packet);
            break;
        case MSG_VIDEO_FRAME:
            handleVideoFrame(socket, packet);
            break;
        case MSG_AUDIO_FRAME:
            handleAudioFrame(socket, packet);
            break;
        case MSG_CONTROL:
            handleControl(socket, packet);
            break;
        default:
            sendErrorResponse(socket, 404, QString("Unknown chat message type: %1").arg(packet.type));
            break;
    }
}

void ChatHandler::handleTextMessage(QTcpSocket* socket, const Packet& packet)
{
    // 验证文本消息格式
    QString text = packet.json.value("text").toString();
    if (text.isEmpty()) {
        sendErrorResponse(socket, 400, "Text message cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("Text message broadcasted from %1")
                         .arg(clientInfo));
}

void ChatHandler::handleDeviceData(QTcpSocket* socket, const Packet& packet)
{
    // 验证设备数据格式
    if (packet.json.isEmpty()) {
        sendErrorResponse(socket, 400, "Device data cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("Device data broadcasted from %1")
                         .arg(clientInfo));
}

void ChatHandler::handleVideoFrame(QTcpSocket* socket, const Packet& packet)
{
    // 验证视频帧数据
    if (packet.bin.isEmpty()) {
        sendErrorResponse(socket, 400, "Video frame data cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("Video frame broadcasted from %1 (%2 bytes)")
                         .arg(clientInfo)
                         .arg(packet.bin.size()));
}

void ChatHandler::handleAudioFrame(QTcpSocket* socket, const Packet& packet)
{
    // 验证音频帧数据
    if (packet.bin.isEmpty()) {
        sendErrorResponse(socket, 400, "Audio frame data cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("Audio frame broadcasted from %1 (%2 bytes)")
                         .arg(clientInfo)
                         .arg(packet.bin.size()));
}

void ChatHandler::handleControl(QTcpSocket* socket, const Packet& packet)
{
    // 验证控制指令
    QString controlType = packet.json.value("control_type").toString();
    if (controlType.isEmpty()) {
        sendErrorResponse(socket, 400, "Control type cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("Control message '%1' broadcasted from %2")
                         .arg(controlType)
                         .arg(clientInfo));
}

void ChatHandler::broadcastToRoom(QTcpSocket* socket, const Packet& packet)
{
    if (!getConnectionManager()) {
        sendErrorResponse(socket, 500, "Connection manager not available");
        return;
    }
    
    ClientContext* context = getClientContext(socket);
    if (!context || context->currentRoom.isEmpty()) {
        sendErrorResponse(socket, 400, "Not in a room");
        return;
    }
    
    // 构建数据包
    QByteArray packetData = buildPacket(packet.type, packet.json, packet.bin);
    
    // 广播到房间内其他成员
    getConnectionManager()->broadcastToRoom(context->currentRoom, packetData, socket);
}
