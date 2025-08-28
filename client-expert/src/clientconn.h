#pragma once
// ===============================================
// 客户端连接封装（两端共用一份拷贝）
// 提供：connectTo(host,port)、sendPacket(type,json,bin)、信号 packetArrived
// ===============================================
#include <QtCore>
#include <QtNetwork>
#include "../../common/protocol.h"

class ClientConn : public QObject {
    Q_OBJECT
public:
    explicit ClientConn(QObject* parent=nullptr);
    void connectTo(const QString& host, quint16 port);
    void send(quint16 type, const QJsonObject& json, const QByteArray& bin = QByteArray());
signals:
    void connected();
    void disconnected();
    void packetArrived(Packet pkt);
private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
private:
    QTcpSocket sock_;
    QByteArray buf_;
};
