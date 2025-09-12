#ifndef NETWORK_STATUS_H
#define NETWORK_STATUS_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include "../../logging/log_manager.h"

// 网络状态枚举
enum NetworkState {
    NETWORK_DISCONNECTED,    // 未连接
    NETWORK_CONNECTING,      // 连接中
    NETWORK_CONNECTED,       // 已连接
    NETWORK_RECONNECTING,    // 重连中
    NETWORK_ERROR           // 连接错误
};

// 网络状态管理 - 负责管理网络连接状态和统计信息
class NetworkStatus : public QObject
{
    Q_OBJECT
public:
    explicit NetworkStatus(QObject *parent = nullptr);
    ~NetworkStatus();

    // 状态查询
    NetworkState getState() const { return currentState_; }
    QString getStateString() const;
    bool isConnected() const { return currentState_ == NETWORK_CONNECTED; }
    bool isConnecting() const { return currentState_ == NETWORK_CONNECTING; }
    bool isReconnecting() const { return currentState_ == NETWORK_RECONNECTING; }
    
    // 连接信息
    QString getServerAddress() const { return serverAddress_; }
    quint16 getServerPort() const { return serverPort_; }
    QDateTime getConnectTime() const { return connectTime_; }
    QDateTime getLastActivityTime() const { return lastActivityTime_; }
    
    // 统计信息
    qint64 getUptime() const;  // 连接时长（毫秒）
    qint64 getInactiveTime() const;  // 空闲时长（毫秒）
    qint64 getTotalBytesSent() const { return totalBytesSent_; }
    qint64 getTotalBytesReceived() const { return totalBytesReceived_; }
    qint64 getMessagesSent() const { return messagesSent_; }
    qint64 getMessagesReceived() const { return messagesReceived_; }
    qint64 getReconnectAttempts() const { return reconnectAttempts_; }
    
    // 状态更新
    void setState(NetworkState state);
    void setServerInfo(const QString& address, quint16 port);
    void updateActivity();
    void addBytesSent(qint64 bytes);
    void addBytesReceived(qint64 bytes);
    void incrementMessagesSent();
    void incrementMessagesReceived();
    void incrementReconnectAttempts();
    
    // 重置统计
    void resetStatistics();
    void resetReconnectAttempts();

signals:
    // 状态变化信号
    void stateChanged(NetworkState oldState, NetworkState newState);
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    void reconnectAttempt(int attempt);

private slots:
    // 定时器处理
    void onUpdateTimer();

private:
    // 私有辅助方法
    void updateUptime();
    void logStateChange(NetworkState oldState, NetworkState newState);
    QString getStateStringFromEnum(NetworkState state);

private:
    // 当前状态
    NetworkState currentState_;
    
    // 连接信息
    QString serverAddress_;
    quint16 serverPort_;
    QDateTime connectTime_;
    QDateTime lastActivityTime_;
    
    // 统计信息
    qint64 totalBytesSent_;
    qint64 totalBytesReceived_;
    qint64 messagesSent_;
    qint64 messagesReceived_;
    qint64 reconnectAttempts_;
    qint64 uptime_;
    
    // 定时器
    QTimer* updateTimer_;
    

};

#endif // NETWORK_STATUS_H
