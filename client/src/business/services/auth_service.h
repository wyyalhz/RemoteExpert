#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include "../models/user.h"
#include "../../logging/log_manager.h"
#include "../../network/client/network_client.h"
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>

// 认证服务 - 负责用户登录、注册、登出等认证相关业务
class AuthService : public QObject
{
    Q_OBJECT

public:
    explicit AuthService(QObject *parent = nullptr);
    ~AuthService();

    // 用户认证
    bool login(const QString& username, const QString& password, int userType);
    bool registerUser(const QString& username, const QString& password, 
                     const QString& email, const QString& phone, int userType);
    bool logout();
    
    // 用户信息管理
    User getCurrentUser() const { return currentUser_; }
    bool isLoggedIn() const { return isLoggedIn_; }
    QString getSessionId() const { return sessionId_; }
    
    // 用户信息更新
    bool updateUserInfo(const User& user);
    bool changePassword(const QString& oldPassword, const QString& newPassword);
    
    // 会话管理
    bool refreshSession();
    bool validateSession();
    
    // 错误处理
    QString getLastError() const { return lastError_; }
    void clearError() { lastError_.clear(); }

signals:
    // 认证状态变化信号
    void loginSuccess(const User& user);
    void loginFailed(const QString& error);
    void logoutSuccess();
    void logoutFailed(const QString& error);
    void registerSuccess(const User& user);
    void registerFailed(const QString& error);
    void sessionExpired();
    void userInfoUpdated(const User& user);



private:
    // 私有辅助方法
    void setCurrentUser(const User& user);
    void clearCurrentUser();
    void setError(const QString& error);
    
    // 网络请求方法（将在网络层实现后调用）
    void sendLoginRequest(const QString& username, const QString& password, int userType);
    void sendRegisterRequest(const QString& username, const QString& password, 
                           const QString& email, const QString& phone, int userType);
    void sendLogoutRequest();
    void sendUpdateUserRequest(const User& user);
    void sendChangePasswordRequest(const QString& oldPassword, const QString& newPassword);
    void sendSessionValidationRequest();
    
    // 响应解析方法
    bool parseLoginResponse(const QJsonObject& response, User& user);
    bool parseRegisterResponse(const QJsonObject& response, User& user);
    bool parseUserInfoResponse(const QJsonObject& response, User& user);

private:
    User currentUser_;
    bool isLoggedIn_;
    QString sessionId_;
    QString lastError_;
    
    // 网络客户端引用
    NetworkClient* networkClient_;
    
public:
    // 设置网络客户端
    void setNetworkClient(NetworkClient* client);

private:

public slots:
    // 网络响应处理
    void onLoginResponse(const QJsonObject& response);
    void onRegisterResponse(const QJsonObject& response);
    void onLogoutResponse(const QJsonObject& response);
    void onUpdateUserResponse(const QJsonObject& response);
    void onChangePasswordResponse(const QJsonObject& response);
    void onSessionValidationResponse(const QJsonObject& response);
};

#endif // AUTH_SERVICE_H
