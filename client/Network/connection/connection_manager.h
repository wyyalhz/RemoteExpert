#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include "../../Logger/log_manager.h"
#include "../../../common/protocol/protocol.h"

// 连接管理器 - 负责TCP连接管理和消息收发
class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionManager(QObject *parent = nullptr);
    ~ConnectionManager();

    // 连接管理
    bool connectToServer(const QString& host, quint16 port);
    bool connectToServer(const QHostAddress& address, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;
    
    // 消息发送
    bool sendMessage(quint16 type, const QJsonObject& data, const QByteArray& binary = QByteArray());
    
    // 状态查询
    QString getLastError() const;
    QString getConnectionInfo() const;
    
    // 重连管理
    void enableAutoReconnect(bool enable, int interval = 5000);
    void disableAutoReconnect();

signals:
    // 连接状态信号
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    
    // 消息接收信号
    void messageReceived(quint16 type, const QJsonObject& data, const QByteArray& binary);

private slots:
    // TCP Socket事件处理
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketReadyRead();
    
    // 重连定时器处理
    void onReconnectTimeout();

private:
    // 私有辅助方法
    void setupSocket();
    void setupReconnectTimer();
    void clearReceiveBuffer();
    void processReceivedData();
    void logConnectionEvent(const QString& event, const QString& details = QString());

private:
    QTcpSocket* socket_;
    QTimer* reconnectTimer_;
    
    // 连接信息
    QString serverHost_;
    quint16 serverPort_;
    bool isConnected_;
    QString lastError_;
    
    // 接收缓冲区
    QByteArray receiveBuffer_;
    
    // 重连设置
    bool autoReconnectEnabled_;
    int reconnectInterval_;
    int reconnectAttempts_;
    static const int MAX_RECONNECT_ATTEMPTS = 10;
    

};

#endif // CONNECTION_MANAGER_H
