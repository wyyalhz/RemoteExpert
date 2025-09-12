#ifndef DB_LOGGER_H
#define DB_LOGGER_H

#include "../../../../common/logging/managers/log_manager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariantMap>

// 数据库日志助手类
class DBLogger
{
public:
    // 基本日志方法
    static void error(const QString &operation, const QSqlError &error);
    static void error(const QString &operation, const QString &message);
    static void warning(const QString &operation, const QString &message);
    static void info(const QString &operation, const QString &message);
    static void debug(const QString &operation, const QString &message);
    
    // 查询相关日志
    static void queryExecution(const QString &operation, const QSqlQuery &query, bool success);
    static void queryExecution(const QString &operation, const QString &sql, bool success, const QString &errorMessage = QString());
    
    // 连接相关日志
    static void connectionEstablished(const QString &connectionName);
    static void connectionFailed(const QString &connectionName, const QString &error);
    static void connectionClosed(const QString &connectionName);
    
    // 事务相关日志
    static void transactionBegin(const QString &operation);
    static void transactionCommit(const QString &operation, bool success);
    static void transactionRollback(const QString &operation, const QString &reason = QString());
    
    // 数据操作日志
    static void dataInserted(const QString &table, int affectedRows);
    static void dataUpdated(const QString &table, int affectedRows, const QString &condition = QString());
    static void dataDeleted(const QString &table, int affectedRows, const QString &condition = QString());
    static void dataSelected(const QString &table, int resultCount, const QString &condition = QString());

private:
    // 格式化SQL错误信息
    static QString formatSqlError(const QSqlError &error);
    
    // 格式化SQL查询信息
    static QString formatSqlQuery(const QSqlQuery &query);
    
    // 获取数据库上下文信息
    static QString getDatabaseContext(const QSqlQuery &query);
};

#endif // DB_LOGGER_H
