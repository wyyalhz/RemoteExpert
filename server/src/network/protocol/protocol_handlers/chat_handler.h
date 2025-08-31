#ifndef CHAT_HANDLER_H
#define CHAT_HANDLER_H

#include "../protocol_handler.h"
#include <QObject>
#include <QTcpSocket>
#include <QThreadPool>
#include <QRunnable>
#include "../../connection_manager.h"

class WorkOrderService;

class ForwardTask:public QRunnable
{
public:
    ForwardTask(QTcpSocket* target,const QByteArray& data);
    void run() override;
private:
    QTcpSocket* m_target;
    QByteArray m_data;
};

// 聊天协议处理器 - 处理聊天相关的消息
class ChatHandler : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit ChatHandler(WorkOrderService* workOrderService, QObject *parent = nullptr);
    ~ChatHandler();

    // 实现基类的消息处理方法
    void handleMessage(QTcpSocket* socket, const Packet& packet) override;

    void joinRoom(QTcpSocket* socket, const QString& roomId);
    void leaveRoom(QTcpSocket* socket);
private:
    // 处理具体的聊天消息
    void handleTextMessage(QTcpSocket* socket, const Packet& packet);
    void handleDeviceData(QTcpSocket* socket, const Packet& packet);
    void handleFileTransfer(QTcpSocket* socket, const Packet& packet);
    void handleScreenshot(QTcpSocket* socket, const Packet& packet);
    void handleVideoFrame(QTcpSocket* socket, const Packet& packet);
    void handleAudioFrame(QTcpSocket* socket, const Packet& packet);
    void handleVideoControl(QTcpSocket* socket, const Packet& packet);
    void handleAudioControl(QTcpSocket* socket, const Packet& packet);
    void handleControl(QTcpSocket* socket, const Packet& packet);
    void handleDeviceControl(QTcpSocket* socket, const Packet& packet);
    void handleSystemControl(QTcpSocket* socket, const Packet& packet);
    
    // 消息转发方法
    void broadcastToRoom(QTcpSocket* socket, const Packet& packet);
    void forwardToRoomParticipants(const QString& roomId, const QByteArray& data, QTcpSocket* excludeSocket = nullptr);
    void handleRealTimeMedia(QTcpSocket* socket, const Packet& packet);

    // 房间管理
    QMap<QTcpSocket*, QString> m_roomMemberships; //  socket -> roomId
    QMap<QString, QSet<QTcpSocket*>> m_roomMembers; // roomId -> sockets

    WorkOrderService* m_workOrderService;
    QThreadPool m_threadPool;
    ConnectionManager* connectionManager_;
};

#endif // CHAT_HANDLER_H
