#ifndef USER_VALIDATOR_H
#define USER_VALIDATOR_H

#include "../exceptions/business_exception.h"
#include "../../data/models/user_model.h"
#include "../../../common/protocol/types/enums.h"
#include <QString>
#include <QJsonObject>

// 用户数据验证器
class UserValidator
{
public:
    // 验证用户注册数据
    static void validateRegistration(const QString& username, const QString& password, 
                                   const QString& email, const QString& phone, int userType);
    
    // 验证用户登录数据
    static void validateLogin(const QString& username, const QString& password, int userType);
    
    // 验证用户模型
    static void validateUserModel(const UserModel& user);
    
    // 验证用户更新数据
    static void validateUserUpdate(const UserModel& user);
    
    // 验证密码强度
    static void validatePassword(const QString& password);
    
    // 验证用户名格式
    static void validateUsername(const QString& username);
    
    // 验证邮箱格式
    static void validateEmail(const QString& email);
    
    // 验证手机号格式
    static void validatePhone(const QString& phone);
    
    // 验证用户类型
    static void validateUserType(int userType);

private:
    // 私有辅助方法
    static bool isValidEmailFormat(const QString& email);
    static bool isValidPhoneFormat(const QString& phone);
    static bool isValidUsernameFormat(const QString& username);
    static bool isStrongPassword(const QString& password);
};

#endif // USER_VALIDATOR_H
