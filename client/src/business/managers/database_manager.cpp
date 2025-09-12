#include "database_manager.h"
#include "../../../common/logging/managers/log_manager.h"
#include <QApplication>
#include <QDir>
#include <QDebug>

// 单例实例获取实现
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QApplication::applicationDirPath() + "/data.db";
    m_db.setDatabaseName(dbPath);
    LogManager::getInstance()->info(LogModule::DATABASE, LogLayer::DATA, 
                                   "DatabaseManager", QString("数据库路径: %1").arg(dbPath));
}

DatabaseManager::~DatabaseManager()
{
    if(m_db.isOpen()){
        m_db.close();
    }
}

bool DatabaseManager::initDatabase()
{
    if(!m_db.open()){
        LogManager::getInstance()->error(LogModule::DATABASE, LogLayer::DATA,
                                        "DatabaseManager", QString("数据库打开失败: %1").arg(m_db.lastError().text()));
        return false;
    }

    // 创建用户表
    QSqlQuery query;
    QString createTableSql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username VARCHAR(255) UNIQUE NOT NULL,"
        "password VARCHAR(255) NOT NULL,"
        "email VARCHAR(255),"
        "phone VARCHAR(20),"
        "user_type INTEGER NOT NULL,"
        "created_time DATETIME DEFAULT CURRENT_TIMESTAMP)";

    if(!query.exec(createTableSql)){
        LogManager::getInstance()->error(LogModule::DATABASE, LogLayer::DATA,
                                        "DatabaseManager", QString("创建用户表失败: %1").arg(query.lastError().text()));
        return false;
    }

    LogManager::getInstance()->info(LogModule::DATABASE, LogLayer::DATA,
                                   "DatabaseManager", "数据库初始化成功");
    return true;
}

bool DatabaseManager::addUser(const QString &username, const QString &password,
                             const QString &email, const QString &phone, int userType)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, email, phone, user_type) "
                 "VALUES (:username, :password, :email, :phone, :userType)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    query.bindValue(":email", email);
    query.bindValue(":phone", phone);
    query.bindValue(":userType", userType);

    if(!query.exec()){
        qDebug() << "Error adding user:" << query.lastError().text();
        return false;
    }

    qDebug() << "User added successfully:" << username;
    return true;
}

bool DatabaseManager::validateUser(const QString &username, const QString &password, int userType)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = :username AND password = :password AND user_type = :userType");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    query.bindValue(":userType", userType);

    if(!query.exec()){
        qDebug() << "Error validating user:" << query.lastError().text();
        return false;
    }

    bool isValid = query.next();
    qDebug() << "User validation result for" << username << ":" << isValid;
    return isValid;
}

bool DatabaseManager::userExists(const QString &username)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if(!query.exec()){
        qDebug() << "Error checking user existence:" << query.lastError().text();
        return false;
    }

    bool exists = query.next();
    qDebug() << "User exists check for" << username << ":" << exists;
    return exists;
}
