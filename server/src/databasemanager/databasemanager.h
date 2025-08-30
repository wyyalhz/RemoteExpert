#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonArray>

// 前向声明
class UserManager;
class WorkOrderManager;
class DBLogger;

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // 初始化数据库
    bool initialize();
    
    // 获取子管理器
    UserManager* userManager() const { return userManager_; }
    WorkOrderManager* workOrderManager() const { return workOrderManager_; }
    DBLogger* logger() const { return logger_; }
    
    // 获取数据库连接
    QSqlDatabase& database() { return db_; }
    bool isOpen() const { return db_.isOpen(); }

private:
    QSqlDatabase db_;
    UserManager* userManager_;
    WorkOrderManager* workOrderManager_;
    DBLogger* logger_;
    
    // 创建数据库表
    bool createTables();
};

#endif // DATABASEMANAGER_H
