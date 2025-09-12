#include "connection_manager.h"
#include "../../logging/log_manager.h"
#include "../../../common/protocol/protocol.h"
#include "../../../common/protocol/serialization/serializer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostInfo>
#include <QNetworkProxy>

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent)
    , socket_(nullptr)
    , reconnectTimer_(nullptr)
    , serverPort_(0)
    , isConnected_(false)
    , autoReconnectEnabled_(false)
    , reconnectInterval_(5000)
    , reconnectAttempts_(0)
{
    setupSocket();
    setupReconnectTimer();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", "连接管理器初始化完成");
}

ConnectionManager::~ConnectionManager()
{
    disconnectFromServer();
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", "连接管理器已销毁");
}

bool ConnectionManager::connectToServer(const QString& host, quint16 port)
{
    if (isConnected_) {
        LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", "已经连接到服务器，先断开现有连接");
        disconnectFromServer();
    }
    
    serverHost_ = host;
    serverPort_ = port;
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                   QString("正在连接到服务器: %1:%2").arg(host, QString::number(port)));
    
    // 解析主机名
    QHostInfo hostInfo = QHostInfo::fromName(host);
    if (hostInfo.error() != QHostInfo::NoError) {
        lastError_ = QString("无法解析主机名: %1").arg(hostInfo.errorString());
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", lastError_);
        return false;
    }
    
    if (hostInfo.addresses().isEmpty()) {
        lastError_ = QString("无法解析主机名: %1").arg(host);
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", lastError_);
        return false;
    }
    
    QHostAddress address = hostInfo.addresses().first();
    return connectToServer(address, port);
}

bool ConnectionManager::connectToServer(const QHostAddress& address, quint16 port)
{
    if (isConnected_) {
        LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", "已经连接到服务器，先断开现有连接");
        disconnectFromServer();
    }
    
    serverHost_ = address.toString();
    serverPort_ = port;
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                   QString("正在连接到服务器: %1:%2").arg(serverHost_, QString::number(port)));
    
    // 连接到服务器
    socket_->connectToHost(address, port);
    
    return true;
}

void ConnectionManager::disconnectFromServer()
{
    if (socket_) {
        socket_->disconnectFromHost();
    }
    
    isConnected_ = false;
    clearReceiveBuffer();
    disableAutoReconnect();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", "已断开服务器连接");
}

bool ConnectionManager::isConnected() const
{
    return isConnected_;
}

bool ConnectionManager::sendMessage(quint16 type, const QJsonObject& data, const QByteArray& binary)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                    QString("开始发送消息: 类型=%1").arg(type));
    
    if (!isConnected_ || !socket_) {
        lastError_ = "未连接到服务器";
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", "发送消息失败: 未连接到服务器");
        return false;
    }
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                    QString("连接状态正常，准备构建数据包"));
    
    try {
        // 使用协议模块构建数据包
        QByteArray packet = buildPacket(type, data, binary);
        
        LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                        QString("数据包构建完成: 大小=%1字节").arg(packet.size()));
        
        // 发送数据
        qint64 bytesWritten = socket_->write(packet);
        
        LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                        QString("Socket写入结果: %1/%2 字节").arg(bytesWritten).arg(packet.size()));
        
        if (bytesWritten != packet.size()) {
            lastError_ = QString("发送数据不完整: %1/%2 字节").arg(bytesWritten).arg(packet.size());
            LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", lastError_);
            return false;
        }
        
        // 确保数据立即发送
        socket_->flush();
        
        LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                        QString("发送消息成功: 类型=%1, 大小=%2字节").arg(type).arg(packet.size()));
        
        return true;
    } catch (const std::exception& e) {
        lastError_ = QString("发送消息异常: %1").arg(e.what());
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", lastError_);
        return false;
    }
}

QString ConnectionManager::getLastError() const
{
    return lastError_;
}

QString ConnectionManager::getConnectionInfo() const
{
    if (!isConnected_) {
        return "未连接";
    }
    
    return QString("已连接到 %1:%2").arg(serverHost_).arg(serverPort_);
}

void ConnectionManager::enableAutoReconnect(bool enable, int interval)
{
    autoReconnectEnabled_ = enable;
    reconnectInterval_ = interval;
    
    if (enable) {
        reconnectAttempts_ = 0;
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                       QString("自动重连已启用，间隔: %1ms").arg(interval));
    } else {
        disableAutoReconnect();
    }
}

void ConnectionManager::disableAutoReconnect()
{
    autoReconnectEnabled_ = false;
    if (reconnectTimer_ && reconnectTimer_->isActive()) {
        reconnectTimer_->stop();
    }
    reconnectAttempts_ = 0;
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", "自动重连已禁用");
}

void ConnectionManager::setupSocket()
{
    socket_ = new QTcpSocket(this);
    
    // 禁用代理，直接连接
    socket_->setProxy(QNetworkProxy::NoProxy);
    
    // 连接信号槽
    connect(socket_, &QTcpSocket::connected, this, &ConnectionManager::onSocketConnected);
    connect(socket_, &QTcpSocket::disconnected, this, &ConnectionManager::onSocketDisconnected);
    connect(socket_, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &ConnectionManager::onSocketError);
    connect(socket_, &QTcpSocket::readyRead, this, &ConnectionManager::onSocketReadyRead);
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", "TCP Socket设置完成");
}

void ConnectionManager::setupReconnectTimer()
{
    reconnectTimer_ = new QTimer(this);
    reconnectTimer_->setSingleShot(true);
    connect(reconnectTimer_, &QTimer::timeout, this, &ConnectionManager::onReconnectTimeout);
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", "重连定时器设置完成");
}

void ConnectionManager::clearReceiveBuffer()
{
    receiveBuffer_.clear();
}

void ConnectionManager::processReceivedData()
{
    QVector<Packet> packets;
    
    // 使用协议模块解析数据包
    if (drainPackets(receiveBuffer_, packets)) {
        for (const Packet& packet : packets) {
            // 验证消息
            if (!MessageValidator::validatePacket(packet)) {
                LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                                 QString("收到无效数据包: 类型=%1").arg(packet.type));
                continue;
            }
            
            // 解析JSON数据
            QJsonObject jsonData = packet.json;
            
            // 发送消息接收信号
            emit messageReceived(packet.type, jsonData, packet.bin);
            
            LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                           QString("解析数据包成功: 类型=%1, JSON大小=%2, 二进制大小=%3")
                                           .arg(packet.type).arg(toJsonBytes(packet.json).size()).arg(packet.bin.size()));
        }
    }
}

void ConnectionManager::logConnectionEvent(const QString& event, const QString& details)
{
    QString message = event;
    if (!details.isEmpty()) {
        message += ": " + details;
    }
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", message);
}

void ConnectionManager::onSocketConnected()
{
    isConnected_ = true;
    lastError_.clear();
    reconnectAttempts_ = 0;
    
    logConnectionEvent("已连接到服务器", QString("%1:%2").arg(serverHost_, QString::number(serverPort_)));
    emit connected();
}

void ConnectionManager::onSocketDisconnected()
{
    isConnected_ = false;
    
    logConnectionEvent("与服务器连接已断开");
    emit disconnected();
    
    // 检查是否需要自动重连
    if (autoReconnectEnabled_ && reconnectAttempts_ < MAX_RECONNECT_ATTEMPTS) {
        reconnectAttempts_++;
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                       QString("准备第 %1 次重连 (最大 %2 次)").arg(reconnectAttempts_).arg(MAX_RECONNECT_ATTEMPTS));
        reconnectTimer_->start(reconnectInterval_);
    } else if (reconnectAttempts_ >= MAX_RECONNECT_ATTEMPTS) {
        LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                         QString("已达到最大重连次数 (%1)，停止重连").arg(MAX_RECONNECT_ATTEMPTS));
        disableAutoReconnect();
    }
}

void ConnectionManager::onSocketError(QAbstractSocket::SocketError error)
{
    QString errorString = socket_->errorString();
    lastError_ = errorString;
    
    LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                    QString("Socket错误: %1 (%2)").arg(errorString).arg(error));
    
    emit connectionError(errorString);
}

void ConnectionManager::onSocketReadyRead()
{
    // 读取所有可用数据
    QByteArray newData = socket_->readAll();
    if (newData.isEmpty()) {
        return;
    }
    
    // 追加到接收缓冲区
    receiveBuffer_.append(newData);
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                    QString("接收到 %1 字节数据").arg(newData.size()));
    
    // 处理接收到的数据
    processReceivedData();
}

void ConnectionManager::onReconnectTimeout()
{
    if (!autoReconnectEnabled_ || reconnectAttempts_ >= MAX_RECONNECT_ATTEMPTS) {
        return;
    }
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "ConnectionManager", 
                                   QString("开始第 %1 次重连尝试").arg(reconnectAttempts_));
    
    // 尝试重新连接
    QHostAddress address(serverHost_);
    if (address.isNull()) {
        // 如果解析失败，尝试通过主机名连接
        connectToServer(serverHost_, serverPort_);
    } else {
        connectToServer(address, serverPort_);
    }
}
