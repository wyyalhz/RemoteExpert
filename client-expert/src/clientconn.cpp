#include "clientconn.h"

ClientConn::ClientConn(QObject* parent) : QObject(parent) {
    connect(&sock_, &QTcpSocket::readyRead, this, &ClientConn::onReadyRead);
    connect(&sock_, &QTcpSocket::connected,  this, &ClientConn::onConnected);
    connect(&sock_, &QTcpSocket::disconnected, this, &ClientConn::onDisconnected);
}

void ClientConn::connectTo(const QString& host, quint16 port) {
    sock_.connectToHost(host, port);
}

void ClientConn::send(quint16 type, const QJsonObject& json, const QByteArray& bin) {
    sock_.write(buildPacket(type, json, bin));
}

void ClientConn::onConnected() { emit connected(); }
void ClientConn::onDisconnected() { emit disconnected(); }

void ClientConn::onReadyRead() {
    buf_.append(sock_.readAll());
    QVector<Packet> pkts;
    if (drainPackets(buf_, pkts)) {
        for (auto& p : pkts) emit packetArrived(p);
    }
}
