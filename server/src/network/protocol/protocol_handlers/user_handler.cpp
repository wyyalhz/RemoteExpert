#include "user_handler.h"
#include "../connection_manager.h"
#include "../logging/network_logger.h"
#include "../../../common/protocol.h"

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
        default:
            sendErrorResponse(socket, 404, QString("Unknown user message type: %1").arg(packet.type));
            break;
    }
}

void UserHandler::handleLogin(QTcpSocket* socket, const QJsonObject& data)
{
    QString username = data.value("username").toString();
    QString password = data.value("password").toString();
    int userType = data.value("user_type").toInt();
    
    // 基本验证
    if (username.isEmpty() || password.isEmpty()) {
        sendErrorResponse(socket, 400, "Username and password cannot be empty");
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
        // 更新客户端认证状态
        updateClientAuthentication(socket, username, true);
        
        // 发送成功响应
        QJsonObject responseData{
            {"username", username},
            {"user_type", userType}
        };
        sendSuccessResponse(socket, "Login successful", responseData);
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::authenticationSuccess(clientInfo, username);
    } else {
        sendErrorResponse(socket, 401, "Invalid username or password");
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::authenticationFailed(clientInfo, "Invalid username or password");
    }
}

void UserHandler::handleRegister(QTcpSocket* socket, const QJsonObject& data)
{
    QString username = data.value("username").toString();
    QString password = data.value("password").toString();
    QString email = data.value("email").toString();
    QString phone = data.value("phone").toString();
    int userType = data.value("user_type").toInt();
    
    // 基本验证
    if (username.isEmpty() || password.isEmpty()) {
        sendErrorResponse(socket, 400, "Username and password cannot be empty");
        return;
    }
    
    if (userType != 0 && userType != 1) {
        sendErrorResponse(socket, 400, "Invalid user type");
        return;
    }
    
    // 调用业务服务进行注册
    bool success = userService_->registerUser(username, password, email, phone, userType);
    
    if (success) {
        sendSuccessResponse(socket, "User registered successfully", QJsonObject{{"username", username}});
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::info("User Handler", 
                           QString("User '%1' registered successfully from %2")
                           .arg(username).arg(clientInfo));
    } else {
        sendErrorResponse(socket, 409, "Username already exists");
        
        QString clientInfo = QString("%1:%2")
                            .arg(socket->peerAddress().toString())
                            .arg(socket->peerPort());
        NetworkLogger::warning("User Handler", 
                              QString("Registration failed for user '%1' from %2")
                              .arg(username).arg(clientInfo));
    }
}

void UserHandler::handleLogout(QTcpSocket* socket, const QJsonObject& data)
{
    ClientContext* context = getClientContext(socket);
    if (!context || !context->isAuthenticated) {
        sendErrorResponse(socket, 400, "Not logged in");
        return;
    }
    
    QString username = context->username;
    
    // 调用业务服务进行登出
    bool success = userService_->logoutUser(username);
    
    if (success) {
        // 更新客户端认证状态
        updateClientAuthentication(socket, "", false);
        
        sendSuccessResponse(socket, "Logout successful");
        
        NetworkLogger::info("User Handler", 
                           QString("User '%1' logged out successfully")
                           .arg(username));
    } else {
        sendErrorResponse(socket, 500, "Logout failed");
    }
}

void UserHandler::updateClientAuthentication(QTcpSocket* socket, const QString& username, bool authenticated)
{
    if (!getConnectionManager()) {
        return;
    }
    
    ClientContext* context = getClientContext(socket);
    if (context) {
        context->username = username;
        context->isAuthenticated = authenticated;
        
        // 更新用户映射
        if (authenticated && !username.isEmpty()) {
            // 添加到用户映射
            // 注意：这里需要访问ConnectionManager的私有成员，可能需要添加公共方法
        } else {
            // 从用户映射中移除
            // 注意：这里需要访问ConnectionManager的私有成员，可能需要添加公共方法
        }
    }
}
