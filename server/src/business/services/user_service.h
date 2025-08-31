#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include "../exceptions/business_exception.h"
#include "../validators/user_validator.h"
#include "../logging/business_logger.h"
#include "../../data/databasemanager.h"
#include "../../data/models/user_model.h"
#include "../../data/repositories/user_repository.h"
#include "../../../common/protocol/types/enums.h"
#include <QObject>
#include <QJsonObject>

// 用户业务服务
class UserService : public QObject
{
    Q_OBJECT
public:
    explicit UserService(DatabaseManager* dbManager, QObject *parent = nullptr);
    ~UserService();

    // 用户认证相关
    bool authenticateUser(const QString& username, const QString& password, int userType);
    bool registerUser(const QString& username, const QString& password, 
                     const QString& email, const QString& phone, int userType);
    bool logoutUser(const QString& username);
    
    // 用户信息管理
    UserModel getUserInfo(const QString& username);
    UserModel getUserInfo(int userId);
    bool updateUserInfo(const UserModel& user);
    bool updatePassword(int userId, const QString& oldPassword, const QString& newPassword);
    bool updateUserType(int userId, int newUserType);
    
    // 用户查询
    QList<UserModel> getUsersByType(int userType, int limit = -1, int offset = 0);
    QList<UserModel> getAllUsers(int limit = -1, int offset = 0);
    int getUserCount(int userType = -1);
    
    // 用户状态检查
    bool userExists(const QString& username);
    bool userExists(int userId);
    int getUserId(const QString& username);
    int getUserType(const QString& username);
    
    // 权限检查
    bool hasPermission(int userId, const QString& operation);
    bool canAccessWorkOrder(int userId, int workOrderId);
    bool canModifyWorkOrder(int userId, int workOrderId);

private:
    DatabaseManager* dbManager_;
    UserRepository* userRepo_;
    
    // 私有辅助方法
    bool validateUserCredentials(const QString& username, const QString& password, int userType);
    void logUserActivity(const QString& operation, const QString& username, bool success, const QString& reason = QString());
    
    // 业务验证方法
    bool validateUserRegistration(const QString& username, const QString& password, 
                                const QString& email, const QString& phone, int userType);
    bool validatePasswordChange(int userId, const QString& oldPassword, const QString& newPassword);
    bool validateUserUpdate(const UserModel& user);
    
    // 权限检查方法
    bool checkUserPermission(int userId, const QString& operation);
    bool checkWorkOrderAccess(int userId, int workOrderId);
    
    // 业务事件触发方法
    void triggerUserRegisteredEvent(const UserModel& user);
    void triggerUserLoginEvent(const UserModel& user);
    void triggerUserLogoutEvent(const UserModel& user);
    void triggerUserPasswordChangedEvent(const UserModel& user);
};

#endif // USER_SERVICE_H
