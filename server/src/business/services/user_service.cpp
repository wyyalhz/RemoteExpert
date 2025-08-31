#include "user_service.h"
#include <QCryptographicHash>

UserService::UserService(DatabaseManager* dbManager, QObject *parent)
    : QObject(parent), dbManager_(dbManager), userRepo_(dbManager->userRepository())
{
    // 创建会话服务
    sessionService_ = new SessionService(dbManager, this);
    BusinessLogger::info("User Service", "User service initialized");
}

UserService::~UserService()
{
    BusinessLogger::info("User Service", "User service destroyed");
}

// 用户认证相关
bool UserService::authenticateUser(const QString& username, const QString& password, int userType)
{
    BusinessLogger::businessOperationStart("User Authentication", username);
    
    try {
        // 数据验证
        UserValidator::validateLogin(username, password, userType);
        
        // 验证用户凭据
        bool success = validateUserCredentials(username, password, userType);
        
        if (success) {
            BusinessLogger::userAuthentication(username, true);
            BusinessLogger::businessOperationSuccess("User Authentication", username);
        } else {
            BusinessLogger::userAuthentication(username, false, "Invalid credentials");
            BusinessLogger::businessOperationFailed("User Authentication", "Invalid credentials");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::userAuthentication(username, false, e.getMessage());
        BusinessLogger::businessOperationFailed("User Authentication", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::userAuthentication(username, false, e.getMessage());
        BusinessLogger::businessOperationFailed("User Authentication", e.getMessage());
        return false;
    }
}

bool UserService::registerUser(const QString& username, const QString& password, 
                             const QString& email, const QString& phone, int userType)
{
    BusinessLogger::businessOperationStart("User Registration", username);
    
    try {
        // 数据验证
        UserValidator::validateRegistration(username, password, email, phone, userType);
        
        // 检查用户是否已存在
        if (userExists(username)) {
            BusinessLogger::userRegistration(username, userType, false, "Username already exists");
            BusinessLogger::businessOperationFailed("User Registration", "Username already exists");
            return false;
        }
        
        // 创建用户模型
        UserModel user;
        user.username = username;
        user.passwordHash = userRepo_->hashPassword(password); // 在业务层哈希密码
        user.email = email;
        user.phone = phone;
        user.userType = userType;
        
        // 保存用户
        int userId = -1;
        bool success = userRepo_->create(user, userId);
        
        if (success) {
            BusinessLogger::userRegistration(username, userType, true);
            BusinessLogger::businessOperationSuccess("User Registration", QString("User ID: %1").arg(userId));
        } else {
            BusinessLogger::userRegistration(username, userType, false, "Database operation failed");
            BusinessLogger::businessOperationFailed("User Registration", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::userRegistration(username, userType, false, e.getMessage());
        BusinessLogger::businessOperationFailed("User Registration", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::userRegistration(username, userType, false, e.getMessage());
        BusinessLogger::businessOperationFailed("User Registration", e.getMessage());
        return false;
    }
}

bool UserService::logoutUser(const QString& username)
{
    BusinessLogger::businessOperationStart("User Logout", username);
    
    try {
        // 获取用户ID
        int userId = getUserId(username);
        if (userId > 0) {
            // 清理用户的所有会话
            removeAllUserSessions(userId);
        }
        
        BusinessLogger::userLogout(username);
        BusinessLogger::businessOperationSuccess("User Logout", username);
        return true;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("User Logout", e.getMessage());
        return false;
    }
}

// 用户信息管理
UserModel UserService::getUserInfo(const QString& username)
{
    BusinessLogger::businessOperationStart("Get User Info", username);
    
    try {
        UserModel user;
        if (userRepo_->findByUsername(username, user)) {
            BusinessLogger::businessOperationSuccess("Get User Info", username);
            return user;
        } else {
            BusinessLogger::businessOperationFailed("Get User Info", "User not found");
            return UserModel(); // 返回空模型
        }
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get User Info", e.getMessage());
        return UserModel();
    }
}

UserModel UserService::getUserInfo(int userId)
{
    BusinessLogger::businessOperationStart("Get User Info", QString::number(userId));
    
    try {
        UserModel user;
        if (userRepo_->findById(userId, user)) {
            BusinessLogger::businessOperationSuccess("Get User Info", QString::number(userId));
            return user;
        } else {
            BusinessLogger::businessOperationFailed("Get User Info", "User not found");
            return UserModel();
        }
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get User Info", e.getMessage());
        return UserModel();
    }
}

bool UserService::updateUserInfo(const UserModel& user)
{
    BusinessLogger::businessOperationStart("Update User Info", user.username);
    
    try {
        // 数据验证
        UserValidator::validateUserUpdate(user);
        
        // 检查用户是否存在
        if (!userExists(user.id)) {
            BusinessLogger::businessOperationFailed("Update User Info", "User not found");
            return false;
        }
        
        // 更新用户信息
        bool success = userRepo_->update(user);
        
        if (success) {
            BusinessLogger::businessOperationSuccess("Update User Info", user.username);
        } else {
            BusinessLogger::businessOperationFailed("Update User Info", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::businessOperationFailed("Update User Info", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Update User Info", e.getMessage());
        return false;
    }
}

bool UserService::updatePassword(int userId, const QString& oldPassword, const QString& newPassword)
{
    BusinessLogger::businessOperationStart("Update Password", QString::number(userId));
    
    try {
        // 验证新密码
        UserValidator::validatePassword(newPassword);
        
        // 获取用户信息
        UserModel user = getUserInfo(userId);
        if (!user.isValid()) {
            BusinessLogger::businessOperationFailed("Update Password", "User not found");
            return false;
        }
        
        // 验证旧密码
        if (!validateUserCredentials(user.username, oldPassword, user.userType)) {
            BusinessLogger::businessOperationFailed("Update Password", "Old password is incorrect");
            return false;
        }
        
        // 哈希新密码并更新
        QString newPasswordHash = userRepo_->hashPassword(newPassword);
        bool success = userRepo_->updatePassword(userId, newPasswordHash);
        
        if (success) {
            BusinessLogger::businessOperationSuccess("Update Password", QString::number(userId));
        } else {
            BusinessLogger::businessOperationFailed("Update Password", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::businessOperationFailed("Update Password", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Update Password", e.getMessage());
        return false;
    }
}

bool UserService::updateUserType(int userId, int newUserType)
{
    BusinessLogger::businessOperationStart("Update User Type", QString::number(userId));
    
    try {
        // 验证用户类型
        UserValidator::validateUserType(newUserType);
        
        // 检查用户是否存在
        if (!userExists(userId)) {
            BusinessLogger::businessOperationFailed("Update User Type", "User not found");
            return false;
        }
        
        // 更新用户类型
        bool success = userRepo_->updateUserType(userId, newUserType);
        
        if (success) {
            BusinessLogger::businessOperationSuccess("Update User Type", QString::number(userId));
        } else {
            BusinessLogger::businessOperationFailed("Update User Type", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::businessOperationFailed("Update User Type", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Update User Type", e.getMessage());
        return false;
    }
}

// 用户查询
QList<UserModel> UserService::getUsersByType(int userType, int limit, int offset)
{
    BusinessLogger::businessOperationStart("Get Users By Type", QString::number(userType));
    
    try {
        QList<UserModel> users = userRepo_->findByUserType(userType);
        BusinessLogger::businessOperationSuccess("Get Users By Type", QString("Found %1 users").arg(users.size()));
        return users;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get Users By Type", e.getMessage());
        return QList<UserModel>();
    }
}

QList<UserModel> UserService::getAllUsers(int limit, int offset)
{
    BusinessLogger::businessOperationStart("Get All Users");
    
    try {
        QList<UserModel> users = userRepo_->findAll(limit, offset);
        BusinessLogger::businessOperationSuccess("Get All Users", QString("Found %1 users").arg(users.size()));
        return users;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get All Users", e.getMessage());
        return QList<UserModel>();
    }
}

int UserService::getUserCount(int userType)
{
    BusinessLogger::businessOperationStart("Get User Count");
    
    try {
        int count = (userType >= 0) ? userRepo_->countByUserType(userType) : userRepo_->countAll();
        BusinessLogger::businessOperationSuccess("Get User Count", QString::number(count));
        return count;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get User Count", e.getMessage());
        return 0;
    }
}

// 用户状态检查
bool UserService::userExists(const QString& username)
{
    return userRepo_->exists(username);
}

bool UserService::userExists(int userId)
{
    UserModel user;
    return userRepo_->findById(userId, user);
}

int UserService::getUserId(const QString& username)
{
    return userRepo_->getUserId(username);
}

int UserService::getUserType(const QString& username)
{
    return userRepo_->getUserType(username);
}

// 权限检查
bool UserService::hasPermission(int userId, const QString& operation)
{
    // 这里可以实现更复杂的权限检查逻辑
    // 目前简单返回true，后续可以扩展
    BusinessLogger::permissionCheck(operation, userId, true);
    return true;
}

bool UserService::canAccessWorkOrder(int userId, int workOrderId)
{
    // 这里可以实现工单访问权限检查逻辑
    // 目前简单返回true，后续可以扩展
    return true;
}

bool UserService::canModifyWorkOrder(int userId, int workOrderId)
{
    // 这里可以实现工单修改权限检查逻辑
    // 目前简单返回true，后续可以扩展
    return true;
}

// 私有辅助方法
bool UserService::validateUserCredentials(const QString& username, const QString& password, int userType)
{
    // 获取用户信息
    UserModel user;
    if (!userRepo_->findByUsername(username, user)) {
        return false;
    }
    
    // 检查用户类型
    if (user.userType != userType) {
        return false;
    }
    
    // 验证密码
    QString inputHash = userRepo_->hashPassword(password);
    return (user.passwordHash == inputHash);
}



void UserService::logUserActivity(const QString& operation, const QString& username, bool success, const QString& reason)
{
    if (success) {
        BusinessLogger::info(operation, QString("User '%1' activity completed successfully").arg(username));
    } else {
        QString message = QString("User '%1' activity failed").arg(username);
        if (!reason.isEmpty()) {
            message += QString(": %1").arg(reason);
        }
        BusinessLogger::warning(operation, message);
    }
}

// 业务验证方法
bool UserService::validateUserRegistration(const QString& username, const QString& password, 
                                        const QString& email, const QString& phone, int userType)
{
    // 检查用户类型是否有效
    if (userType != USER_TYPE_NORMAL && userType != USER_TYPE_EXPERT) {
        BusinessLogger::validationFailed("User Registration", "userType", "Invalid user type");
        return false;
    }
    
    // 检查用户名是否已被使用
    if (userExists(username)) {
        BusinessLogger::validationFailed("User Registration", "username", "Username already exists");
        return false;
    }
    
    // 检查邮箱是否已被使用（如果提供了邮箱）
    if (!email.isEmpty()) {
        // 这里可以添加邮箱唯一性检查
    }
    
    return true;
}

bool UserService::validatePasswordChange(int userId, const QString& oldPassword, const QString& newPassword)
{
    // 获取用户信息
    UserModel user = getUserInfo(userId);
    if (!user.isValid()) {
        BusinessLogger::validationFailed("Password Change", "userId", "User not found");
        return false;
    }
    
    // 验证旧密码
    QString oldHash = userRepo_->hashPassword(oldPassword);
    if (user.passwordHash != oldHash) {
        BusinessLogger::validationFailed("Password Change", "oldPassword", "Old password is incorrect");
        return false;
    }
    
    // 检查新密码是否与旧密码相同
    if (oldPassword == newPassword) {
        BusinessLogger::validationFailed("Password Change", "newPassword", "New password must be different from old password");
        return false;
    }
    
    return true;
}

bool UserService::validateUserUpdate(const UserModel& user)
{
    // 检查用户是否存在
    if (!userExists(user.id)) {
        BusinessLogger::validationFailed("User Update", "userId", "User not found");
        return false;
    }
    
    // 检查用户名是否已被其他用户使用
    UserModel existingUser;
    if (userRepo_->findByUsername(user.username, existingUser) && existingUser.id != user.id) {
        BusinessLogger::validationFailed("User Update", "username", "Username already exists");
        return false;
    }
    
    return true;
}

// 权限检查方法
bool UserService::checkUserPermission(int userId, const QString& operation)
{
    // 这里应该调用权限管理器
    // 目前简单返回true，后续可以扩展
    return true;
}

bool UserService::checkWorkOrderAccess(int userId, int workOrderId)
{
    // 这里应该调用工单服务检查访问权限
    // 目前简单返回true，后续可以扩展
    return true;
}

// 业务事件触发方法
void UserService::triggerUserRegisteredEvent(const UserModel& user)
{
    // 这里应该调用事件分发器
    BusinessLogger::eventTriggered("user.registered", "UserService", 
        QJsonObject{
            {"userId", user.id}, 
            {"username", user.username},
            {"userType", user.userType}
        });
}

void UserService::triggerUserLoginEvent(const UserModel& user)
{
    // 这里应该调用事件分发器
    BusinessLogger::eventTriggered("user.login", "UserService", 
        QJsonObject{
            {"userId", user.id}, 
            {"username", user.username}
        });
}

void UserService::triggerUserLogoutEvent(const UserModel& user)
{
    // 这里应该调用事件分发器
    BusinessLogger::eventTriggered("user.logout", "UserService", 
        QJsonObject{
            {"userId", user.id}, 
            {"username", user.username}
        });
}

void UserService::triggerUserPasswordChangedEvent(const UserModel& user)
{
    // 这里应该调用事件分发器
    BusinessLogger::eventTriggered("user.password.changed", "UserService", 
        QJsonObject{
            {"userId", user.id}, 
            {"username", user.username}
        });
}

// 会话管理相关方法实现
QString UserService::createUserSession(int userId, const QString& roomId, int timeoutMinutes)
{
    if (!sessionService_) {
        BusinessLogger::error("User Service", "Session service not available");
        return QString();
    }
    
    return sessionService_->createSession(userId, roomId, timeoutMinutes);
}

bool UserService::updateUserSessionActivity(const QString& sessionId)
{
    if (!sessionService_) {
        BusinessLogger::error("User Service", "Session service not available");
        return false;
    }
    
    return sessionService_->updateSessionActivity(sessionId);
}

bool UserService::expireUserSession(const QString& sessionId)
{
    if (!sessionService_) {
        BusinessLogger::error("User Service", "Session service not available");
        return false;
    }
    
    return sessionService_->expireSession(sessionId);
}

bool UserService::isUserSessionValid(const QString& sessionId)
{
    if (!sessionService_) {
        BusinessLogger::error("User Service", "Session service not available");
        return false;
    }
    
    return sessionService_->isSessionValid(sessionId);
}

bool UserService::isUserInRoom(int userId, const QString& roomId)
{
    if (!sessionService_) {
        BusinessLogger::error("User Service", "Session service not available");
        return false;
    }
    
    return sessionService_->isUserInRoom(userId, roomId);
}

QList<SessionModel> UserService::getUserSessions(int userId)
{
    if (!sessionService_) {
        BusinessLogger::error("User Service", "Session service not available");
        return QList<SessionModel>();
    }
    
    return sessionService_->getUserSessions(userId);
}

bool UserService::removeUserSession(const QString& sessionId)
{
    if (!sessionService_) {
        BusinessLogger::error("User Service", "Session service not available");
        return false;
    }
        
    return sessionService_->removeSession(sessionId);
}

bool UserService::removeAllUserSessions(int userId)
{
    if (!sessionService_) {
        BusinessLogger::error("User Service", "Session service not available");
        return false;
    }
    
    return sessionService_->removeUserSessions(userId);
}

SessionService* UserService::getSessionService() const
{
    return sessionService_;
}
