#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "../base/db_base.h"
#include <QCryptographicHash>

// 用户类型枚举
enum UserType {
    FACTORY_USER = 0,    // 工厂端用户
    EXPERT_USER = 1      // 专家端用户
};

class UserManager : public DBBase
{
    Q_OBJECT
public:
    explicit UserManager(QObject *parent = nullptr);
    ~UserManager();

    // 用户验证
    bool validateUser(const QString &username, const QString &password, int userType);
    
    // 用户注册
    bool registerUser(const QString &username, const QString &password, 
                     const QString &email = "", const QString &phone = "", int userType = 0);
    
    // 获取用户信息
    int getUserId(const QString &username);
    int getUserType(const QString &username);
    QString getUserEmail(const QString &username);
    QString getUserPhone(const QString &username);
    bool userExists(const QString &username);

private:
    // 密码哈希
    QString hashPassword(const QString &password);
};

#endif // USER_MANAGER_H
