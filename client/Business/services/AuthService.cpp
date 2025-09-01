#include "AuthService.h"
#include "../../common/protocol/builders/message_builder.h"
#include "../../Network/client/network_client.h"

AuthService::AuthService(QObject *parent)
    : QObject(parent)
    , isLoggedIn_(false)
    , networkClient_(nullptr)
{
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", "认证服务初始化完成");
}

void AuthService::setNetworkClient(NetworkClient* client)
{
    networkClient_ = client;
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", "网络客户端已设置");
}

AuthService::~AuthService()
{
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", "认证服务销毁");
}

bool AuthService::login(const QString& username, const QString& password, int userType)
{
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", QString("用户尝试登录: %1").arg(username));
    
    // 输入验证
    if (username.isEmpty() || password.isEmpty()) {
        setError("用户名和密码不能为空");
        LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                          "AuthService", "登录失败: 用户名或密码为空");
        emit loginFailed(lastError_);
        return false;
    }
    
    if (username.length() < 3) {
        setError("用户名长度至少3个字符");
        LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                          "AuthService", QString("登录失败: 用户名长度不足 - %1").arg(username));
        emit loginFailed(lastError_);
        return false;
    }
    
    // 发送登录请求（将在网络层实现后调用）
    sendLoginRequest(username, password, userType);
    
    // 暂时返回false，等待网络响应
    return false;
}

bool AuthService::registerUser(const QString& username, const QString& password, 
                              const QString& email, const QString& phone, int userType)
{
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", QString("用户尝试注册: %1").arg(username));
    
    // 输入验证
    if (username.isEmpty() || password.isEmpty()) {
        setError("用户名和密码不能为空");
        LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                          "AuthService", "注册失败: 用户名或密码为空");
        emit registerFailed(lastError_);
        return false;
    }
    
    if (username.length() < 3) {
        setError("用户名长度至少3个字符");
        LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                          "AuthService", QString("注册失败: 用户名长度不足 - %1").arg(username));
        emit registerFailed(lastError_);
        return false;
    }
    
    if (password.length() < 6) {
        setError("密码长度至少6个字符");
        LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                          "AuthService", "注册失败: 密码长度不足");
        emit registerFailed(lastError_);
        return false;
    }
    
    // 发送注册请求
    sendRegisterRequest(username, password, email, phone, userType);
    
    return true;
}

bool AuthService::logout()
{
    if (!isLoggedIn_) {
        LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                          "AuthService", "登出失败: 用户未登录");
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", QString("用户登出: %1").arg(currentUser_.getUsername()));
    
    // 发送登出请求
    sendLogoutRequest();
    
    return true;
}

bool AuthService::updateUserInfo(const User& user)
{
    if (!isLoggedIn_) {
        setError("用户未登录");
        return false;
    }
    
    if (!user.isValid()) {
        setError(user.getValidationError());
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", QString("更新用户信息: %1").arg(user.getUsername()));
    
    // 发送更新用户信息请求
    sendUpdateUserRequest(user);
    
    return true;
}

bool AuthService::changePassword(const QString& oldPassword, const QString& newPassword)
{
    if (!isLoggedIn_) {
        setError("用户未登录");
        return false;
    }
    
    if (oldPassword.isEmpty() || newPassword.isEmpty()) {
        setError("旧密码和新密码不能为空");
        return false;
    }
    
    if (newPassword.length() < 6) {
        setError("新密码长度至少6个字符");
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", QString("用户修改密码: %1").arg(currentUser_.getUsername()));
    
    // 发送修改密码请求
    sendChangePasswordRequest(oldPassword, newPassword);
    
    return true;
}

bool AuthService::refreshSession()
{
    if (!isLoggedIn_ || sessionId_.isEmpty()) {
        return false;
    }
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "刷新会话");
    
    // 发送会话验证请求
    sendSessionValidationRequest();
    
    return true;
}

bool AuthService::validateSession()
{
    if (!isLoggedIn_ || sessionId_.isEmpty()) {
        return false;
    }
    
    // 发送会话验证请求
    sendSessionValidationRequest();
    
    return true;
}

void AuthService::setCurrentUser(const User& user)
{
    currentUser_ = user;
    isLoggedIn_ = true;
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", QString("设置当前用户: %1").arg(user.getUsername()));
}

void AuthService::clearCurrentUser()
{
    currentUser_ = User();
    isLoggedIn_ = false;
    sessionId_.clear();
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", "清除当前用户");
}

void AuthService::setError(const QString& error)
{
    lastError_ = error;
    LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", QString("错误: %1").arg(error));
}

// 网络请求方法
void AuthService::sendLoginRequest(const QString& username, const QString& password, int userType)
{
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "开始发送登录请求");
    
    if (!networkClient_) {
        setError("网络客户端未初始化");
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", "网络客户端未初始化");
        emit loginFailed(lastError_);
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", QString("检查网络连接状态: %1").arg(networkClient_->isConnected() ? "已连接" : "未连接"));
    
    if (!networkClient_->isConnected()) {
        setError("未连接到服务器");
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", "未连接到服务器");
        emit loginFailed(lastError_);
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "准备调用网络客户端发送登录请求");
    
    // 通过网络客户端发送登录请求
    bool success = networkClient_->sendLoginRequest(username, password, userType);
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", QString("网络客户端发送登录请求结果: %1").arg(success ? "成功" : "失败"));
    
    if (!success) {
        setError("发送登录请求失败");
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", "发送登录请求失败");
        emit loginFailed(lastError_);
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "登录请求已发送");
}

void AuthService::sendRegisterRequest(const QString& username, const QString& password, 
                                    const QString& email, const QString& phone, int userType)
{
    if (!networkClient_) {
        setError("网络客户端未初始化");
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", "网络客户端未初始化");
        emit registerFailed(lastError_);
        return;
    }
    
    if (!networkClient_->isConnected()) {
        setError("未连接到服务器");
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", "未连接到服务器");
        emit registerFailed(lastError_);
        return;
    }
    
    // 通过网络客户端发送注册请求
    bool success = networkClient_->sendRegisterRequest(username, password, email, phone, userType);
    if (!success) {
        setError("发送注册请求失败");
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", "发送注册请求失败");
        emit registerFailed(lastError_);
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "注册请求已发送");
}

void AuthService::sendLogoutRequest()
{
    if (!networkClient_) {
        setError("网络客户端未初始化");
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", "网络客户端未初始化");
        emit logoutFailed(lastError_);
        return;
    }
    
    if (!networkClient_->isConnected()) {
        setError("未连接到服务器");
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", "未连接到服务器");
        emit logoutFailed(lastError_);
        return;
    }
    
    // 通过网络客户端发送登出请求
    bool success = networkClient_->sendLogoutRequest();
    if (!success) {
        setError("发送登出请求失败");
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", "发送登出请求失败");
        emit logoutFailed(lastError_);
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "登出请求已发送");
}

void AuthService::sendUpdateUserRequest(const User& user)
{
    // TODO: 构建更新用户信息消息并发送
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "发送更新用户信息请求");
}

void AuthService::sendChangePasswordRequest(const QString& oldPassword, const QString& newPassword)
{
    // TODO: 构建修改密码消息并发送
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "发送修改密码请求");
}

void AuthService::sendSessionValidationRequest()
{
    // TODO: 构建会话验证消息并发送
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "发送会话验证请求");
}

// 网络响应处理方法（占位符，将在网络层实现后完善）
void AuthService::onLoginResponse(const QJsonObject& response)
{
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", QString("收到登录响应: %1").arg(QJsonDocument(response).toJson(QJsonDocument::Compact).constData()));
    
    // 解析登录响应
    User user;
    if (parseLoginResponse(response, user)) {
        LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                       "AuthService", QString("登录响应解析成功，用户: %1").arg(user.getUsername()));
        setCurrentUser(user);
        emit loginSuccess(user);
    } else {
        LogManager::getInstance()->error(LogModule::USER, LogLayer::BUSINESS, 
                                        "AuthService", QString("登录响应解析失败: %1").arg(lastError_));
        emit loginFailed(lastError_);
    }
}

void AuthService::onRegisterResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "收到注册响应");
    
    // TODO: 解析注册响应
    User user;
    if (parseRegisterResponse(response, user)) {
        emit registerSuccess(user);
    } else {
        emit registerFailed(lastError_);
    }
}

void AuthService::onLogoutResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "收到登出响应");
    
    // TODO: 解析登出响应
    clearCurrentUser();
    emit logoutSuccess();
}

void AuthService::onUpdateUserResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "收到更新用户信息响应");
    
    // TODO: 解析更新用户信息响应
    User user;
    if (parseUserInfoResponse(response, user)) {
        setCurrentUser(user);
        emit userInfoUpdated(user);
    }
}

void AuthService::onChangePasswordResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "收到修改密码响应");
    
    // TODO: 解析修改密码响应
}

void AuthService::onSessionValidationResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "收到会话验证响应");
    
    // TODO: 解析会话验证响应
    if (response.contains("valid") && !response["valid"].toBool()) {
        clearCurrentUser();
        emit sessionExpired();
    }
}

// 响应解析方法（占位符，将在网络层实现后完善）
bool AuthService::parseLoginResponse(const QJsonObject& response, User& user)
{
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "解析登录响应");
    
    // 检查响应状态 - 服务器返回格式：{"code": 0, "message": "Login successful", ...}
    if (response.contains("code") && response["code"].toInt() != 0) {
        QString error = response.value("message").toString();
        if (error.isEmpty()) error = "登录失败";
        setError(error);
        return false;
    }
    
    // 解析用户信息 - 服务器直接返回用户信息，不在data字段中
    if (response.contains("username")) {
        user.setUsername(response.value("username").toString());
        user.setUserType(response.value("user_type").toInt());
        
        // 可选字段
        if (response.contains("email")) {
            user.setEmail(response.value("email").toString());
        }
        if (response.contains("phone")) {
            user.setPhone(response.value("phone").toString());
        }
        if (response.contains("id")) {
            user.setId(response.value("id").toInt());
        }
        
        // 设置会话ID（如果服务器返回了的话）
        if (response.contains("session_id")) {
            sessionId_ = response["session_id"].toString();
        }
        
        LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                       "AuthService", QString("登录响应解析成功: %1").arg(user.getUsername()));
        return true;
    }
    
    setError("响应数据格式错误");
    return false;
}

bool AuthService::parseRegisterResponse(const QJsonObject& response, User& user)
{
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "解析注册响应");
    
    // 兼容服务器响应格式 {"code": 0, "message": "..."}
    if (response.contains("code")) {
        int code = response.value("code").toInt();
        if (code != 0) {
            QString error = response.value("message").toString();
            if (error.isEmpty()) error = "注册失败";
            setError(error);
            return false;
        }
    }
    
    // 注册成功，创建用户对象
    // 注意：注册响应通常不包含完整的用户信息，我们创建一个基本用户对象
    user.setUsername(""); // 用户名将在调用处设置
    user.setUserType(0);  // 用户类型将在调用处设置
    
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "AuthService", "注册响应解析成功");
    return true;
}

bool AuthService::parseUserInfoResponse(const QJsonObject& response, User& user)
{
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "AuthService", "解析用户信息响应");
    
    // 检查响应状态
    if (response.contains("success") && !response["success"].toBool()) {
        QString error = response.value("message").toString();
        if (error.isEmpty()) error = "获取用户信息失败";
        setError(error);
        return false;
    }
    
    // 解析用户信息
    if (response.contains("data")) {
        QJsonObject userData = response["data"].toObject();
        
        user.setId(userData.value("id").toInt());
        user.setUsername(userData.value("username").toString());
        user.setEmail(userData.value("email").toString());
        user.setPhone(userData.value("phone").toString());
        user.setUserType(userData.value("user_type").toInt());
        
        LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                       "AuthService", QString("用户信息响应解析成功: %1").arg(user.getUsername()));
        return true;
    }
    
    setError("响应数据格式错误");
    return false;
}
