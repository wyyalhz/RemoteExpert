#include "network_status.h"
#include "../../../common/logging/managers/log_manager.h"
#include <QDateTime>

NetworkStatus::NetworkStatus(QObject *parent)
    : QObject(parent)
    , currentState_(NETWORK_DISCONNECTED)
    , serverPort_(0)
    , totalBytesSent_(0)
    , totalBytesReceived_(0)
    , messagesSent_(0)
    , messagesReceived_(0)
    , reconnectAttempts_(0)
    , uptime_(0)
    , updateTimer_(nullptr)
{
    // 创建定时器
    updateTimer_ = new QTimer(this);
    updateTimer_->setInterval(1000); // 1秒更新一次
    connect(updateTimer_, &QTimer::timeout, this, &NetworkStatus::onUpdateTimer);
    updateTimer_->start();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkStatus", "网络状态管理器初始化完成");
}

NetworkStatus::~NetworkStatus()
{
    if (updateTimer_) {
        updateTimer_->stop();
    }
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkStatus", "网络状态管理器已销毁");
}

QString NetworkStatus::getStateString() const
{
    switch (currentState_) {
        case NETWORK_DISCONNECTED:
            return "未连接";
        case NETWORK_CONNECTING:
            return "连接中";
        case NETWORK_CONNECTED:
            return "已连接";
        case NETWORK_RECONNECTING:
            return "重连中";
        case NETWORK_ERROR:
            return "连接错误";
        default:
            return "未知状态";
    }
}

qint64 NetworkStatus::getUptime() const
{
    if (currentState_ != NETWORK_CONNECTED || !connectTime_.isValid()) {
        return 0;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    return connectTime_.msecsTo(now);
}

qint64 NetworkStatus::getInactiveTime() const
{
    if (!lastActivityTime_.isValid()) {
        return 0;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    return lastActivityTime_.msecsTo(now);
}

void NetworkStatus::setState(NetworkState state)
{
    if (currentState_ != state) {
        NetworkState oldState = currentState_;
        currentState_ = state;
        
        logStateChange(oldState, state);
        emit stateChanged(oldState, state);
        
        // 根据状态变化发送特定信号
        switch (state) {
            case NETWORK_CONNECTED:
                connectTime_ = QDateTime::currentDateTime();
                lastActivityTime_ = connectTime_;
                resetReconnectAttempts();
                emit connected();
                break;
            case NETWORK_DISCONNECTED:
                uptime_ = 0;
                emit disconnected();
                break;
            case NETWORK_ERROR:
                emit connectionError("连接错误");
                break;
            case NETWORK_RECONNECTING:
                incrementReconnectAttempts();
                emit reconnectAttempt(reconnectAttempts_);
                break;
            default:
                break;
        }
    }
}

void NetworkStatus::setServerInfo(const QString& address, quint16 port)
{
    serverAddress_ = address;
    serverPort_ = port;
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkStatus", 
                    QString("服务器信息已设置: %1:%2").arg(address).arg(port));
}

void NetworkStatus::updateActivity()
{
    lastActivityTime_ = QDateTime::currentDateTime();
}

void NetworkStatus::addBytesSent(qint64 bytes)
{
    totalBytesSent_ += bytes;
    updateActivity();
}

void NetworkStatus::addBytesReceived(qint64 bytes)
{
    totalBytesReceived_ += bytes;
    updateActivity();
}

void NetworkStatus::incrementMessagesSent()
{
    messagesSent_++;
    updateActivity();
}

void NetworkStatus::incrementMessagesReceived()
{
    messagesReceived_++;
    updateActivity();
}

void NetworkStatus::incrementReconnectAttempts()
{
    reconnectAttempts_++;
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkStatus", 
                    QString("重连尝试次数: %1").arg(reconnectAttempts_));
}

void NetworkStatus::resetStatistics()
{
    totalBytesSent_ = 0;
    totalBytesReceived_ = 0;
    messagesSent_ = 0;
    messagesReceived_ = 0;
    uptime_ = 0;
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkStatus", "网络统计信息已重置");
}

void NetworkStatus::resetReconnectAttempts()
{
    reconnectAttempts_ = 0;
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkStatus", "重连尝试次数已重置");
}

void NetworkStatus::updateUptime()
{
    if (currentState_ == NETWORK_CONNECTED && connectTime_.isValid()) {
        uptime_ = getUptime();
    }
}

void NetworkStatus::logStateChange(NetworkState oldState, NetworkState newState)
{
    QString oldStateStr = getStateStringFromEnum(oldState);
    QString newStateStr = getStateStringFromEnum(newState);
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkStatus", 
                    QString("网络状态变化: %1 -> %2").arg(oldStateStr).arg(newStateStr));
}

QString NetworkStatus::getStateStringFromEnum(NetworkState state)
{
    switch (state) {
        case NETWORK_DISCONNECTED:
            return "未连接";
        case NETWORK_CONNECTING:
            return "连接中";
        case NETWORK_CONNECTED:
            return "已连接";
        case NETWORK_RECONNECTING:
            return "重连中";
        case NETWORK_ERROR:
            return "连接错误";
        default:
            return "未知状态";
    }
}

void NetworkStatus::onUpdateTimer()
{
    updateUptime();
}
