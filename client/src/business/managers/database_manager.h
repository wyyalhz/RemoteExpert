#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    // 获取单例实例的静态方法
    static DatabaseManager& instance();

    // 删除拷贝构造函数和赋值运算符以防止复制
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool initDatabase();
    bool addUser(const QString &username, const QString &password,
                 const QString &email, const QString &phone, int userType);
    bool validateUser(const QString &username, const QString &password, int userType);
    bool userExists(const QString &username);

private:
    // 私有构造函数
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
