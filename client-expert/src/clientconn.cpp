#include "clientconn.h"

// 构造函数：创建socket并挂载事件回调
ClientConn::ClientConn(QObject* parent) : QObject(parent) {
    connect(&sock_, &QTcpSocket::readyRead, this, &ClientConn::onReadyRead);
    connect(&sock_, &QTcpSocket::connected,  this, &ClientConn::onConnected);
    connect(&sock_, &QTcpSocket::disconnected, this, &ClientConn::onDisconnected);
}

// 连接到指定主机端口
void ClientConn::connectTo(const QString& host, quint16 port) {
    sock_.connectToHost(host, port);
}

// 发送协议包：封包为 [type|len|json|bin] 写入socket
void ClientConn::send(quint16 type, const QJsonObject& json, const QByteArray& bin) {
    sock_.write(buildPacket(type, json, bin));
}

// socket已连接 -> 转发connected信号
void ClientConn::onConnected() { emit connected(); }
// socket断开 -> 转发disconnected信号
void ClientConn::onDisconnected() { emit disconnected(); }

// 收到数据 -> 累加到缓冲并尽可能解析成Packet，逐个发出
void ClientConn::onReadyRead() {
    buf_.append(sock_.readAll());
    QVector<Packet> pkts;
    if (drainPackets(buf_, pkts)) {
        for (auto& p : pkts) emit packetArrived(p);
    }
}
