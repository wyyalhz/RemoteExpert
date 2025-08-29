#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "../base/db_base.h"
#include <QCryptographicHash>

class UserManager : public DBBase
{
    Q_OBJECT
public:
    explicit UserManager(QObject *parent = nullptr);
    ~UserManager();

    // 用户验证
    bool validateUser(const QString &username, const QString &password);
    
    // 用户注册
    bool registerUser(const QString &username, const QString &password, const QString &role = "operator");
    
    // 获取用户信息
    int getUserId(const QString &username);
    QString getUserRole(const QString &username);
    bool userExists(const QString &username);

private:
    // 密码哈希
    QString hashPassword(const QString &password);
};

#endif // USER_MANAGER_H
