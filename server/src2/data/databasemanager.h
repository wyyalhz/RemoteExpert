#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QDir>
#include <QDebug>

// 前向声明
class WorkOrderRepository;
class UserRepository;
class SessionRepository;

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // 初始化数据库
    bool initialize();
    
    // 获取各个Repository实例
    WorkOrderRepository* workOrderRepository() const;
    UserRepository* userRepository() const;
    SessionRepository* sessionRepository() const;
    
    // 事务管理
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // 数据库连接管理
    QSqlDatabase& database();
    bool isConnected() const;
    
private:
    QSqlDatabase db_;
    WorkOrderRepository* workOrderRepo_;
    UserRepository* userRepo_;
    SessionRepository* sessionRepo_;
    
    // 私有方法
    bool createTables();
    bool createWorkOrderTables();
    bool createUserTables();
    bool createSessionTables();
    bool ensureDatabaseDirectory();
};

#endif // DATABASEMANAGER_H
