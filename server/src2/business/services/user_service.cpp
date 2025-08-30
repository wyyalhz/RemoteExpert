#include "user_service.h"

UserService::UserService(DatabaseManager* dbManager, QObject *parent)
    : QObject(parent), dbManager_(dbManager), userRepo_(dbManager->userRepository())
{
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
        user.passwordHash = password; // 将在repository中哈希
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
    
    // 这里可以添加会话清理等逻辑
    BusinessLogger::userLogout(username);
    BusinessLogger::businessOperationSuccess("User Logout", username);
    
    return true;
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
        
        // 更新密码
        bool success = userRepo_->updatePassword(userId, newPassword);
        
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
    return userRepo_->validateUser(username, password, userType);
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
