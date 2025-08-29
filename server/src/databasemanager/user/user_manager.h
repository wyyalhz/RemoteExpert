#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>

class UserManager : public QObject
{
    Q_OBJECT
public:
    explicit UserManager(QObject *parent = nullptr);
    ~UserManager();

    // 设置数据库连接
    void setDatabase(QSqlDatabase &db) { db_ = db; }
    
    // 用户验证
    bool validateUser(const QString &username, const QString &password);
    
    // 用户注册
    bool registerUser(const QString &username, const QString &password, const QString &role = "operator");
    
    // 获取用户信息
    int getUserId(const QString &username);
    QString getUserRole(const QString &username);
    bool userExists(const QString &username);

private:
    QSqlDatabase db_;
    
    // 密码哈希
    QString hashPassword(const QString &password);
};

#endif // USER_MANAGER_H
