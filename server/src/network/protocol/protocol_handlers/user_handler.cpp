#include "user_handler.h"
#include "../connection_manager.h"
#include "../logging/network_logger.h"
#include "../../../common/protocol/protocol.h"
#include <QDateTime>

UserHandler::UserHandler(UserService* userService, QObject *parent)
    : ProtocolHandler(parent)
    , userService_(userService)
{
}

UserHandler::~UserHandler()
{
}

void UserHandler::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    switch (packet.type) {
        case MSG_LOGIN:
            handleLogin(socket, packet.json);
            break;
        case MSG_REGISTER:
            handleRegister(socket, packet.json);
            break;
        case MSG_LOGOUT:
            handleLogout(socket, packet.json);
            break;
        case MSG_HEARTBEAT:
            handleHeartbeat(socket, packet.json);
            break;
        default:
            sendErrorResponse(socket, 404, QString("Unknown user message type: %1").arg(packet.type));
            break;
    }
}

void UserHandler::handleLogin(QTcpSocket* socket, const QJsonObject& data)
{
    // 使用MessageValidator验证登录消息
    QString validationError;
    if (!MessageValidator::validateLoginMessage(data, validationError)) {
        sendErrorResponse(socket, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析登录消息
    QString username, password;
    int userType;
    if (!MessageParser::parseLoginMessage(data, username, password, userType)) {
        sendErrorResponse(socket, 400, "Invalid login message format");
        return;
    }
    
    // 检查是否已经登录
    ClientContext* context = getClientContext(socket);
    if (context && context->isAuthenticated) {
        sendErrorResponse(socket, 400, "Already logged in");
        return;
    }
    
    // 调用业务服务进行认证
    bool success = userService_->authenticateUser(username, password, userType);
    
    if (success) {
        // 获取用户ID
        int userId = userService_->getUserId(username);
        
        // 更新客户端认证状态
        updateClientAuthentication(socket, username, true);
        
        // 创建用户会话（临时房间，后续加入工单时会更新）
        QString tempRoomId = QString("temp_%1").arg(userId);
        if (getConnectionManager()) {
            getConnectionManager()->createSessionForUser(socket, userId, tempRoomId);
        }
        
        // 使用MessageBuilder构建成功响应
        QJsonObject responseData = MessageBuilder::buildSuccessResponse("Login successful", 
            QJsonObject{{"username", username}, {"user_type", userType}, {"id", userId}});
        sendResponse(socket, MSG_LOGIN, responseData);
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::authenticationSuccess(clientInfo, username);
    } else {
        sendErrorResponse(socket, 401, "Invalid username or password");
        // [FIX]为了让客户端统一通过 MSG_LOGIN(不再用sendErrorResponse)处理登录失败
        QJsonObject fail = MessageBuilder::buildErrorResponse(401, "Invalid username or password");
        sendResponse(socket, MSG_LOGIN, fail);
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::authenticationFailed(clientInfo, "Invalid username or password");
    }
}

void UserHandler::handleLogout(QTcpSocket* socket, const QJsonObject& data)
{
    // 获取客户端上下文
    ClientContext* context = getClientContext(socket);
    if (context && !context->username.isEmpty()) {
        // 调用业务服务登出（会清理会话）
        userService_->logoutUser(context->username);
    }
    
    // 更新客户端认证状态
    updateClientAuthentication(socket, QString(), false);
    
    // 发送成功响应
    sendSuccessResponse(socket, "Logout successful", QJsonObject{});
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::info("User Handler", 
                       QString("User logged out from %1")
                       .arg(clientInfo));
}

void UserHandler::handleHeartbeat(QTcpSocket* socket, const QJsonObject& data)
{
    // 发送心跳响应
    QJsonObject responseData = MessageBuilder::buildHeartbeatResponse(
        QDateTime::currentMSecsSinceEpoch());
    sendSuccessResponse(socket, "Heartbeat received", responseData);
    
    QString clientInfo = QString("%1:%2")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort());
    NetworkLogger::debug("User Handler", 
                        QString("Heartbeat from %1")
                        .arg(clientInfo));
}

void UserHandler::handleRegister(QTcpSocket* socket, const QJsonObject& data)
{
    // 使用MessageValidator验证注册消息
    QString validationError;
    if (!MessageValidator::validateRegisterMessage(data, validationError)) {
        sendErrorResponse(socket, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析注册消息
    QString username, password, email, phone;
    int userType;
    if (!MessageParser::parseRegisterMessage(data, username, password, email, phone, userType)) {
        sendErrorResponse(socket, 400, "Invalid register message format");
        return;
    }
    
    // 调用业务服务进行注册
    bool success = userService_->registerUser(username, password, email, phone, userType);
    
    if (success) {
        sendSuccessResponse(socket, "Registration successful", QJsonObject{});
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::info("User Handler", 
                           QString("User '%1' registered successfully from %2")
                           .arg(username)
                           .arg(clientInfo));
    } else {
        sendErrorResponse(socket, 500, "Registration failed");
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::error("User Handler", 
                            QString("Failed to register user '%1' from %2")
                            .arg(username)
                            .arg(clientInfo));
    }
}

void UserHandler::updateClientAuthentication(QTcpSocket* socket, const QString& username, bool authenticated)
{
    ConnectionManager* manager = getConnectionManager();
    if (!manager) {
        return;
    }
    
    ClientContext* context = manager->getContext(socket);
    if (context) {
        context->isAuthenticated = authenticated;
        context->username = authenticated ? username : QString();
        
        if (authenticated) {
            // 将用户添加到用户映射中
            manager->addUserSocket(username, socket);
        } else {
            // 从用户映射中移除用户
            manager->removeUserSocket(username);
        }
    }
}
