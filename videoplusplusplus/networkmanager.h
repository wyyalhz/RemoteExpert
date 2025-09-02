#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H


#pragma once
#include <QObject>
#include <QTcpSocket>
#include "../common/protocol/serialization/serializer.h"
#include "../common/protocol/builders/message_builder.h"

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject* parent = nullptr);

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();

    void sendPacket(quint16 type, const QJsonObject& json,
                    const QByteArray& bin = QByteArray());

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& err);

    void packetReceived(const Packet& pkt);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket m_socket;
    QByteArray m_buffer; // 接收缓冲区
};

#endif // NETWORKMANAGER_H
