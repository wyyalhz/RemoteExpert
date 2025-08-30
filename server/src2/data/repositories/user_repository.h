#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include "../base/db_base.h"
#include "../models/user_model.h"
#include <QList>
#include <QCryptographicHash>

class UserRepository : public DBBase
{
    Q_OBJECT
public:
    explicit UserRepository(QObject *parent = nullptr);
    ~UserRepository();

    // 基础CRUD操作
    bool create(const UserModel& user, int& userId);
    bool findById(int userId, UserModel& user);
    bool findByUsername(const QString& username, UserModel& user);
    bool update(const UserModel& user);
    bool remove(int userId);
    
    // 用户管理
    bool validateUser(const QString& username, const QString& password, int userType);
    bool registerUser(const QString& username, const QString& password, 
                     const QString& email = "", const QString& phone = "", int userType = 0);
    bool updatePassword(int userId, const QString& newPasswordHash);
    bool updateUserType(int userId, int userType);
    
    // 查询操作
    QList<UserModel> findByUserType(int userType);
    QList<UserModel> findAll(int limit = -1, int offset = 0);
    
    // 统计查询
    int countByUserType(int userType);
    int countAll();
    bool exists(const QString& username);
    
    // 获取用户信息
    int getUserId(const QString& username);
    int getUserType(const QString& username);
    QString getUserEmail(const QString& username);
    QString getUserPhone(const QString& username);

private:
    // 私有辅助方法
    UserModel mapToModel(const QSqlRecord& record);
    bool executeUserQuery(QSqlQuery& query, const QString& operation);
    QString hashPassword(const QString& password);
};

#endif // USER_REPOSITORY_H
