#include "network_server.h"
#include "logging/network_logger.h"
#include "../../../common/protocol/protocol.h"

NetworkServer::NetworkServer(QObject *parent)
    : QObject(parent)
    , tcpServer_(nullptr)
    , connectionManager_(nullptr)
    , messageRouter_(nullptr)
    , userHandler_(nullptr)
    , workOrderHandler_(nullptr)
    , chatHandler_(nullptr)
    , userService_(nullptr)
    , workOrderService_(nullptr)
{
}

NetworkServer::~NetworkServer()
{
    stop();
    
    // 清理资源
    delete chatHandler_;
    delete workOrderHandler_;
    delete userHandler_;
    delete messageRouter_;
    delete connectionManager_;
    delete tcpServer_;
}

bool NetworkServer::initialize(UserService* userService, WorkOrderService* workOrderService)
{
    if (!userService || !workOrderService) {
        NetworkLogger::error("Network Server", "User service or work order service is null");
        return false;
    }
    
    userService_ = userService;
    workOrderService_ = workOrderService;
    
    // 创建网络组件
    tcpServer_ = new TCPServer(this);
    connectionManager_ = new ConnectionManager(this);
    messageRouter_ = new MessageRouter(this);
    
    // 创建协议处理器
    userHandler_ = new UserHandler(userService_, this);
    workOrderHandler_ = new WorkOrderHandler(workOrderService_, this);
    chatHandler_ = new ChatHandler(this);
    
    // 设置组件间的连接
    setupConnections();
    
    // 注册消息处理器
    registerMessageHandlers();
    
    NetworkLogger::info("Network Server", "Network server initialized successfully");
    
    return true;
}

bool NetworkServer::start(quint16 port)
{
    return start(QHostAddress::Any, port);
}

bool NetworkServer::start(const QHostAddress &address, quint16 port)
{
    if (!tcpServer_) {
        NetworkLogger::error("Network Server", "TCP server not initialized");
        return false;
    }
    
    bool success = tcpServer_->start(address, port);
    if (success) {
        NetworkLogger::serverStarted(address, port);
    } else {
        NetworkLogger::serverError(QString("Failed to start network server on %1:%2")
                                  .arg(address.toString()).arg(port));
    }
    
    return success;
}

void NetworkServer::stop()
{
    if (tcpServer_) {
        tcpServer_->stop();
        NetworkLogger::info("Network Server", "Network server stopped");
    }
}

bool NetworkServer::isRunning() const
{
    return tcpServer_ ? tcpServer_->isListening() : false;
}

QString NetworkServer::getLastError() const
{
    return tcpServer_ ? tcpServer_->lastError() : QString();
}

int NetworkServer::getConnectionCount() const
{
    return connectionManager_ ? connectionManager_->getConnectionCount() : 0;
}

int NetworkServer::getRoomCount() const
{
    // 这里需要实现房间计数功能
    // 暂时返回0
    return 0;
}

void NetworkServer::registerMessageHandlers()
{
    if (!messageRouter_) {
        return;
    }
    
    // 注册用户相关消息处理器
    messageRouter_->registerHandler(MSG_LOGIN, userHandler_);
    messageRouter_->registerHandler(MSG_REGISTER, userHandler_);
    
    // 注册工单相关消息处理器
    messageRouter_->registerHandler(MSG_CREATE_WORKORDER, workOrderHandler_);
    messageRouter_->registerHandler(MSG_JOIN_WORKORDER, workOrderHandler_);
    messageRouter_->registerHandler(MSG_LEAVE_WORKORDER, workOrderHandler_);
    messageRouter_->registerHandler(MSG_UPDATE_WORKORDER, workOrderHandler_);
    messageRouter_->registerHandler(MSG_LIST_WORKORDERS, workOrderHandler_);
    
    // 注册聊天相关消息处理器
    messageRouter_->registerHandler(MSG_TEXT, chatHandler_);
    messageRouter_->registerHandler(MSG_DEVICE_DATA, chatHandler_);
    messageRouter_->registerHandler(MSG_FILE_TRANSFER, chatHandler_);
    messageRouter_->registerHandler(MSG_SCREENSHOT, chatHandler_);
    messageRouter_->registerHandler(MSG_VIDEO_FRAME, chatHandler_);
    messageRouter_->registerHandler(MSG_AUDIO_FRAME, chatHandler_);
    messageRouter_->registerHandler(MSG_VIDEO_CONTROL, chatHandler_);
    messageRouter_->registerHandler(MSG_AUDIO_CONTROL, chatHandler_);
    messageRouter_->registerHandler(MSG_CONTROL, chatHandler_);
    messageRouter_->registerHandler(MSG_DEVICE_CONTROL, chatHandler_);
    messageRouter_->registerHandler(MSG_SYSTEM_CONTROL, chatHandler_);
    
    NetworkLogger::info("Network Server", "Message handlers registered successfully");
}

void NetworkServer::setupConnections()
{
    if (!tcpServer_ || !connectionManager_ || !messageRouter_) {
        return;
    }
    
    // 设置TCP服务器和连接管理器的连接
    tcpServer_->setConnectionManager(connectionManager_);
    
    // 设置连接管理器和消息路由器的连接
    connectionManager_->setMessageRouter(messageRouter_);
    
    // 设置协议处理器的连接管理器
    userHandler_->setConnectionManager(connectionManager_);
    workOrderHandler_->setConnectionManager(connectionManager_);
    chatHandler_->setConnectionManager(connectionManager_);
    
    NetworkLogger::info("Network Server", "Component connections established");
}

void NetworkServer::onServerStarted()
{
    NetworkLogger::info("Network Server", "Server started event received");
}

void NetworkServer::onServerStopped()
{
    NetworkLogger::info("Network Server", "Server stopped event received");
}

void NetworkServer::onConnectionCountChanged()
{
    NetworkLogger::debug("Network Server", 
                         QString("Connection count changed: %1").arg(getConnectionCount()));
}
