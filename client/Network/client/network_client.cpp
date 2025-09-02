#include "network_client.h"
#include "../connection/connection_manager.h"
#include "../protocol/handlers/message_handler.h"
#include "../../Logger/log_manager.h"
#include "../../../common/protocol/protocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , connectionManager_(nullptr)
    , messageHandler_(nullptr)
    , heartbeatTimer_(nullptr)
    , isConnected_(false)
{
    // 创建连接管理器
    connectionManager_ = new ConnectionManager(this);
    
    // 创建消息处理器
    messageHandler_ = new MessageHandler(this);
    
    // 创建心跳定时器
    heartbeatTimer_ = new QTimer(this);
    heartbeatTimer_->setSingleShot(false);
    
    // 设置连接
    setupConnections();
    setupMessageHandlers();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", "网络客户端初始化完成");
}

NetworkClient::~NetworkClient()
{
    stopHeartbeat();
    disconnectFromServer();
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", "网络客户端已销毁");
}

bool NetworkClient::connectToServer(const QString& host, quint16 port)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                   QString("正在连接到服务器: %1:%2").arg(host).arg(port));
    
    if (isConnected_) {
        LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", "已经连接到服务器，先断开现有连接");
        disconnectFromServer();
    }
    
    bool success = connectionManager_->connectToServer(host, port);
    if (success) {
        connectionStatus_ = QString("正在连接到 %1:%2").arg(host).arg(port);
    } else {
        lastError_ = connectionManager_->getLastError();
        connectionStatus_ = "连接失败";
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                        QString("连接失败: %1").arg(lastError_));
    }
    
    return success;
}

bool NetworkClient::connectToServer(const QHostAddress& address, quint16 port)
{
    return connectToServer(address.toString(), port);
}

void NetworkClient::disconnectFromServer()
{
    if (isConnected_) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", "正在断开服务器连接");
        stopHeartbeat();
        connectionManager_->disconnectFromServer();
    }
}

bool NetworkClient::isConnected() const
{
    return isConnected_;
}

bool NetworkClient::sendMessage(quint16 type, const QJsonObject& data, const QByteArray& binary)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                    QString("准备发送消息: 类型=%1").arg(type));
    
    if (!isConnected_) {
        lastError_ = "未连接到服务器";
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", "发送消息失败: 未连接到服务器");
        return false;
    }
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                    QString("网络已连接，准备调用连接管理器发送消息"));
    
    logMessage(type, data, true);
    bool result = connectionManager_->sendMessage(type, data, binary);
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                    QString("连接管理器发送消息结果: %1").arg(result ? "成功" : "失败"));
    
    return result;
}

bool NetworkClient::sendLoginRequest(const QString& username, const QString& password, int userType)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                    QString("准备发送登录请求: 用户名=%1, 用户类型=%2").arg(username).arg(userType));
    
    QJsonObject data = MessageBuilder::buildLoginMessage(username, password, userType);
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                    QString("登录消息构建完成: %1").arg(QJsonDocument(data).toJson(QJsonDocument::Compact).constData()));
    
    bool result = sendMessage(MSG_LOGIN, data);
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                    QString("登录请求发送结果: %1").arg(result ? "成功" : "失败"));
    
    return result;
}

bool NetworkClient::sendRegisterRequest(const QString& username, const QString& password, 
                                       const QString& email, const QString& phone, int userType)
{
    QJsonObject data = MessageBuilder::buildRegisterMessage(username, password, email, phone, userType);
    return sendMessage(MSG_REGISTER, data);
}

bool NetworkClient::sendLogoutRequest()
{
    QJsonObject data;
    data["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    return sendMessage(MSG_LOGOUT, data);
}

bool NetworkClient::sendCreateTicketRequest(const QString& title, const QString& description, 
                                           const QString& priority, const QString& category, 
                                           const QString& expertUsername, const QJsonObject& deviceInfo)
{
    // 将字符串优先级转换为数字优先级
    int priorityValue = convertPriorityToInt(priority);
    QJsonObject data = MessageBuilder::buildCreateWorkOrderMessage(title, description, 
                                                                 priorityValue, category, expertUsername, deviceInfo);
    return sendMessage(MSG_CREATE_WORKORDER, data);
}

bool NetworkClient::sendJoinTicketRequest(const QString& ticketId, const QString& role)
{
    QJsonObject data = MessageBuilder::buildJoinWorkOrderMessage(ticketId, role);
    return sendMessage(MSG_JOIN_WORKORDER, data);
}

bool NetworkClient::sendLeaveTicketRequest(const QString& ticketId)
{
    QJsonObject data;
    data["workorderId"] = ticketId;
    data["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    return sendMessage(MSG_LEAVE_WORKORDER, data);
}

bool NetworkClient::sendGetTicketListRequest(const QString& status, int limit, int offset)
{
    QJsonObject data;
    if (!status.isEmpty()) {
        data["status"] = status;
    }
    if (limit > 0) {
        data["limit"] = limit;
    }
    if (offset > 0) {
        data["offset"] = offset;
    }
    data["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    return sendMessage(MSG_LIST_WORKORDERS, data);
}

bool NetworkClient::sendGetTicketDetailRequest(const QString& ticketId, int userId, int userType)
{
    QJsonObject data = MessageBuilder::buildGetWorkOrderMessage(ticketId, userId, userType);
    return sendMessage(MSG_GET_WORKORDER, data);
}

bool NetworkClient::sendUpdateTicketRequest(const QJsonObject& ticketData)
{
    QJsonObject data = ticketData;
    data["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    return sendMessage(MSG_UPDATE_WORKORDER, data);
}

bool NetworkClient::sendUpdateStatusRequest(const QString& ticketId, const QString& newStatus)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                   QString("发送状态更新请求: 工单%1 -> %2").arg(ticketId).arg(newStatus));
    
    QJsonObject data;
    data["ticket_id"] = ticketId;  // 使用ticket_id字段，与服务器端保持一致
    data["status"] = newStatus;
    data["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                   QString("状态更新请求数据: %1").arg(QString(QJsonDocument(data).toJson())));
    
    return sendMessage(MSG_UPDATE_WORKORDER, data);
}

bool NetworkClient::sendAssignTicketRequest(int ticketId, int assigneeId)
{
    QJsonObject data;
    data["workorderId"] = QString::number(ticketId);
    data["assigneeId"] = assigneeId;
    data["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    return sendMessage(MSG_UPDATE_WORKORDER, data);
}

QString NetworkClient::getLastError() const
{
    return lastError_;
}

QString NetworkClient::getConnectionStatus() const
{
    return connectionStatus_;
}

void NetworkClient::startHeartbeat(int interval)
{
    if (heartbeatTimer_->isActive()) {
        heartbeatTimer_->stop();
    }
    
    heartbeatTimer_->setInterval(interval);
    heartbeatTimer_->start();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                   QString("心跳已启动，间隔: %1ms").arg(interval));
}

void NetworkClient::stopHeartbeat()
{
    if (heartbeatTimer_->isActive()) {
        heartbeatTimer_->stop();
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", "心跳已停止");
    }
}

void NetworkClient::setupConnections()
{
    // 连接管理器信号
    connect(connectionManager_, &ConnectionManager::connected, 
            this, &NetworkClient::onConnected);
    connect(connectionManager_, &ConnectionManager::disconnected, 
            this, &NetworkClient::onDisconnected);
    connect(connectionManager_, &ConnectionManager::connectionError, 
            this, &NetworkClient::onConnectionError);
    connect(connectionManager_, &ConnectionManager::messageReceived, 
            this, &NetworkClient::onMessageReceived);
    
    // 心跳定时器信号
    connect(heartbeatTimer_, &QTimer::timeout, 
            this, &NetworkClient::onHeartbeatTimeout);
}

void NetworkClient::setupMessageHandlers()
{
    // 设置消息处理器
    messageHandler_->setNetworkClient(this);
}

void NetworkClient::logMessage(quint16 type, const QJsonObject& data, bool isOutgoing)
{
    QString direction = isOutgoing ? "发送" : "接收";
    QString messageType = QString::number(type);
    
    // 根据消息类型获取描述
    switch (type) {
        case MSG_LOGIN: messageType = "登录"; break;
        case MSG_REGISTER: messageType = "注册"; break;
        case MSG_LOGOUT: messageType = "登出"; break;
        case MSG_HEARTBEAT: messageType = "心跳"; break;
        case MSG_CREATE_WORKORDER: messageType = "创建工单"; break;
        case MSG_JOIN_WORKORDER: messageType = "加入工单"; break;
        case MSG_LEAVE_WORKORDER: messageType = "离开工单"; break;
        case MSG_UPDATE_WORKORDER: messageType = "更新工单"; break;
        case MSG_LIST_WORKORDERS: messageType = "获取工单列表"; break;
        case MSG_GET_WORKORDER: messageType = "获取工单详情"; break;
        case MSG_DELETE_WORKORDER: messageType = "删除工单"; break;
        case MSG_TEXT: messageType = "文本消息"; break;
        case MSG_SERVER_EVENT: messageType = "服务器事件"; break;
        case MSG_ERROR: messageType = "错误消息"; break;
        case MSG_NOTIFICATION: messageType = "通知消息"; break;
        default: messageType = QString("未知消息(%1)").arg(type); break;
    }
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                    QString("%1 %2消息: %3").arg(direction, messageType, QJsonDocument(data).toJson(QJsonDocument::Compact)));
}

void NetworkClient::onConnected()
{
    isConnected_ = true;
    connectionStatus_ = "已连接";
    lastError_.clear();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", "已连接到服务器");
    emit connected();
}

void NetworkClient::onDisconnected()
{
    isConnected_ = false;
    connectionStatus_ = "已断开";
    stopHeartbeat();
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", "与服务器连接已断开");
    emit disconnected();
}

void NetworkClient::onConnectionError(const QString& error)
{
    isConnected_ = false;
    connectionStatus_ = "连接错误";
    lastError_ = error;
    stopHeartbeat();
    
    LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                    QString("连接错误: %1").arg(error));
    emit connectionError(error);
}

void NetworkClient::onMessageReceived(quint16 type, const QJsonObject& data, const QByteArray& binary)
{
    logMessage(type, data, false);
    
    // 根据消息类型分发到相应的信号
    switch (type) {
        case MSG_LOGIN:
            emit loginResponse(data);
            break;
        case MSG_REGISTER:
            emit registerResponse(data);
            break;
        case MSG_LOGOUT:
            emit logoutResponse(data);
            break;
        case MSG_CREATE_WORKORDER:
            emit createTicketResponse(data);
            break;
        case MSG_JOIN_WORKORDER:
            emit joinTicketResponse(data);
            break;
        case MSG_LEAVE_WORKORDER:
            emit leaveTicketResponse(data);
            break;
        case MSG_LIST_WORKORDERS:
            emit getTicketListResponse(data);
            break;
        case MSG_GET_WORKORDER:
            emit getTicketDetailResponse(data);
            break;
        case MSG_UPDATE_WORKORDER:
            emit updateTicketResponse(data);
            break;
        case MSG_DELETE_WORKORDER:
            emit deleteTicketResponse(data);
            break;
        case MSG_SERVER_EVENT:
            // 检查是否是登录相关的服务器事件
            if (data.contains("message") && data["message"].toString().contains("Login successful")) {
                LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                               "检测到登录成功的服务器事件，转发为登录响应");
                emit loginResponse(data);
            } else {
                emit serverEvent(data);
            }
            break;
        case MSG_ERROR:
            emit errorMessage(data);
            break;
        case MSG_NOTIFICATION:
            emit notification(data);
            break;
        case MSG_HEARTBEAT:
            // 心跳响应，不需要特殊处理
            break;
        default:
            LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                             QString("收到未知消息类型: %1").arg(type));
            break;
    }
}

void NetworkClient::onHeartbeatTimeout()
{
    if (isConnected_) {
        QJsonObject heartbeatData;
        heartbeatData["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        sendMessage(MSG_HEARTBEAT, heartbeatData);
    }
}

int NetworkClient::convertPriorityToInt(const QString& priority)
{
    // 将字符串优先级转换为数字优先级
    if (priority == "low" || priority == "低") {
        return 1;
    } else if (priority == "normal" || priority == "中" || priority == "medium") {
        return 2;
    } else if (priority == "high" || priority == "高") {
        return 3;
    } else if (priority == "urgent" || priority == "紧急") {
        return 3; // 紧急也映射为高优先级
    } else {
        // 默认返回中等优先级
        LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "NetworkClient", 
                                         QString("未知优先级值: %1，使用默认优先级2").arg(priority));
        return 2;
    }
}
