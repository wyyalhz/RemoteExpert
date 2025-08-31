#include "chat_handler.h"
#include "../connection_manager.h"
#include "../logging/network_logger.h"
#include "../../../common/protocol/protocol.h"

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
        case MSG_FILE_TRANSFER:
            handleFileTransfer(socket, packet);
            break;
        case MSG_SCREENSHOT:
            handleScreenshot(socket, packet);
            break;
        case MSG_VIDEO_FRAME:
            handleVideoFrame(socket, packet);
            break;
        case MSG_AUDIO_FRAME:
            handleAudioFrame(socket, packet);
            break;
        case MSG_VIDEO_CONTROL:
            handleVideoControl(socket, packet);
            break;
        case MSG_AUDIO_CONTROL:
            handleAudioControl(socket, packet);
            break;
        case MSG_CONTROL:
            handleControl(socket, packet);
            break;
        case MSG_DEVICE_CONTROL:
            handleDeviceControl(socket, packet);
            break;
        case MSG_SYSTEM_CONTROL:
            handleSystemControl(socket, packet);
            break;
        default:
            sendErrorResponse(socket, 404, QString("Unknown chat message type: %1").arg(packet.type));
            break;
    }
}

void ChatHandler::handleTextMessage(QTcpSocket* socket, const Packet& packet)
{
    // 使用MessageValidator验证文本消息
    QString validationError;
    if (!MessageValidator::validateTextMessage(packet.json, validationError)) {
        sendErrorResponse(socket, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析文本消息
    QString roomId, text, messageId;
    qint64 timestamp;
    if (!MessageParser::parseTextMessage(packet.json, roomId, text, timestamp, messageId)) {
        sendErrorResponse(socket, 400, "Invalid text message format");
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
    // 使用MessageValidator验证设备数据消息
    QString validationError;
    if (!MessageValidator::validateDeviceDataMessage(packet.json, validationError)) {
        sendErrorResponse(socket, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析设备数据消息
    QString roomId, deviceType;
    QJsonObject deviceData;
    qint64 timestamp;
    if (!MessageParser::parseDeviceDataMessage(packet.json, roomId, deviceType, deviceData, timestamp)) {
        sendErrorResponse(socket, 400, "Invalid device data message format");
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
    // 使用MessageValidator验证视频帧消息
    QString validationError;
    if (!MessageValidator::validateVideoFrameMessage(packet.json, validationError)) {
        sendErrorResponse(socket, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析视频帧消息
    QString roomId, frameId;
    int width, height, fps;
    qint64 timestamp;
    if (!MessageParser::parseVideoFrameMessage(packet.json, roomId, frameId, width, height, fps, timestamp)) {
        sendErrorResponse(socket, 400, "Invalid video frame message format");
        return;
    }
    
    // 验证二进制数据
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
    // 使用MessageValidator验证音频帧消息
    QString validationError;
    if (!MessageValidator::validateAudioFrameMessage(packet.json, validationError)) {
        sendErrorResponse(socket, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析音频帧消息
    QString roomId, frameId;
    int sampleRate, channels;
    qint64 timestamp;
    if (!MessageParser::parseAudioFrameMessage(packet.json, roomId, frameId, sampleRate, channels, timestamp)) {
        sendErrorResponse(socket, 400, "Invalid audio frame message format");
        return;
    }
    
    // 验证二进制数据
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

void ChatHandler::handleFileTransfer(QTcpSocket* socket, const Packet& packet)
{
    // 验证文件传输数据
    if (packet.bin.isEmpty()) {
        sendErrorResponse(socket, 400, "File transfer data cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("File transfer broadcasted from %1 (%2 bytes)")
                         .arg(clientInfo)
                         .arg(packet.bin.size()));
}

void ChatHandler::handleScreenshot(QTcpSocket* socket, const Packet& packet)
{
    // 验证截图数据
    if (packet.bin.isEmpty()) {
        sendErrorResponse(socket, 400, "Screenshot data cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("Screenshot broadcasted from %1 (%2 bytes)")
                         .arg(clientInfo)
                         .arg(packet.bin.size()));
}

void ChatHandler::handleVideoControl(QTcpSocket* socket, const Packet& packet)
{
    // 验证视频控制数据
    if (packet.json.isEmpty()) {
        sendErrorResponse(socket, 400, "Video control data cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("Video control broadcasted from %1")
                         .arg(clientInfo));
}

void ChatHandler::handleAudioControl(QTcpSocket* socket, const Packet& packet)
{
    // 验证音频控制数据
    if (packet.json.isEmpty()) {
        sendErrorResponse(socket, 400, "Audio control data cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("Audio control broadcasted from %1")
                         .arg(clientInfo));
}

void ChatHandler::handleDeviceControl(QTcpSocket* socket, const Packet& packet)
{
    // 验证设备控制数据
    if (packet.json.isEmpty()) {
        sendErrorResponse(socket, 400, "Device control data cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("Device control broadcasted from %1")
                         .arg(clientInfo));
}

void ChatHandler::handleSystemControl(QTcpSocket* socket, const Packet& packet)
{
    // 验证系统控制数据
    if (packet.json.isEmpty()) {
        sendErrorResponse(socket, 400, "System control data cannot be empty");
        return;
    }
    
    // 广播到房间
    broadcastToRoom(socket, packet);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("Chat Handler", 
                         QString("System control broadcasted from %1")
                         .arg(clientInfo));
}

void ChatHandler::handleControl(QTcpSocket* socket, const Packet& packet)
{
    // 使用MessageValidator验证控制消息
    QString validationError;
    if (!MessageValidator::validateControlMessage(packet.json, validationError)) {
        sendErrorResponse(socket, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析控制消息
    QString roomId, controlType, target;
    QJsonObject params;
    qint64 timestamp;
    if (!MessageParser::parseControlMessage(packet.json, roomId, controlType, target, params, timestamp)) {
        sendErrorResponse(socket, 400, "Invalid control message format");
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
