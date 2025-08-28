#pragma once
// ===============================================
// server/src/roomhub.h
// 最小服务器：监听TCP，维护“房间(roomId) -> 客户端列表”
// 功能：转发同一roomId内的消息（先支持 MSG_JOIN_WORKORDER / MSG_TEXT）
// ===============================================
#include <QtCore>
#include <QtNetwork>
#include "../../common/protocol.h"

struct ClientCtx {
    QTcpSocket* sock = nullptr;
    QString user;    // 用户名，仅用于日志/展示
    QString roomId;  // 当前加入的房间；空字符串表示未加入任何房间
};

class RoomHub : public QObject {
    Q_OBJECT
public:
    explicit RoomHub(QObject* parent=nullptr);
    bool start(quint16 port);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpServer server_;
    // 连接索引：socket -> ClientCtx
    QHash<QTcpSocket*, ClientCtx*> clients_;
    // 房间索引：roomId -> sockets（允许多人）
    QMultiHash<QString, QTcpSocket*> rooms_;

    void handlePacket(ClientCtx* c, const Packet& p);
    void joinRoom(ClientCtx* c, const QString& roomId);
    void broadcastToRoom(const QString& roomId,
                         const QByteArray& packet,
                         QTcpSocket* except = nullptr);
};
