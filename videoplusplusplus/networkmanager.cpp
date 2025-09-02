#include "networkmanager.h"
#include <QDebug>

NetworkManager::NetworkManager(QObject* parent):QObject(parent)
{
    connect(&m_socket,&QTcpSocket::readyRead,this,&NetworkManager::onReadyRead);
    connect(&m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(&m_socket,
               QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
               this, &NetworkManager::onError);
}

void NetworkManager::connectToServer(const QString &host, quint16 port)
{
    m_socket.connectToHost(host,port);
}

void NetworkManager::disconnectFromServer()
{
    m_socket.disconnectFromHost();
}

void NetworkManager::sendPacket(quint16 type, const QJsonObject &json, const QByteArray &bin)
{
    QByteArray packet = buildPacket(type,json,bin);
    m_socket.write(packet);
}

void NetworkManager::onReadyRead()
{
    m_buffer.append(m_socket.readAll());

    QVector<Packet>packets;
    if(drainPackets(m_buffer,packets))
    {
        for(const auto& pkt:packets)
        {
            emit packetReceived(pkt);
        }
    }
}

void NetworkManager::onConnected()
{
    emit connected();
}

void NetworkManager::onDisconnected()
{
    emit disconnected();
}

void NetworkManager::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit errorOccurred(m_socket.errorString());
}

