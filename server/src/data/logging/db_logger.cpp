#include "db_logger.h"
#include <QSqlDatabase>
#include <QSqlDriver>

// 基本日志方法实现
void DBLogger::error(const QString &operation, const QSqlError &error)
{
    QString errorMessage = formatSqlError(error);
    LOG_ERROR(LogModule::DATABASE, LogLayer::DATA, operation, errorMessage);
}

void DBLogger::error(const QString &operation, const QString &message)
{
    LOG_ERROR(LogModule::DATABASE, LogLayer::DATA, operation, message);
}

void DBLogger::warning(const QString &operation, const QString &message)
{
    LOG_WARNING(LogModule::DATABASE, LogLayer::DATA, operation, message);
}

void DBLogger::info(const QString &operation, const QString &message)
{
    LOG_INFO(LogModule::DATABASE, LogLayer::DATA, operation, message);
}

void DBLogger::debug(const QString &operation, const QString &message)
{
    LOG_DEBUG(LogModule::DATABASE, LogLayer::DATA, operation, message);
}

// 查询相关日志实现
void DBLogger::queryExecution(const QString &operation, const QSqlQuery &query, bool success)
{
    QString sqlInfo = formatSqlQuery(query);
    QString context = getDatabaseContext(query);
    
    if (success) {
        LOG_INFO(LogModule::DATABASE, LogLayer::DATA, operation, 
                 QString("SQL: %1, Context: %2").arg(sqlInfo).arg(context));
    } else {
        QString errorMessage = formatSqlError(query.lastError());
        LOG_ERROR(LogModule::DATABASE, LogLayer::DATA, operation, 
                  QString("SQL: %1, Error: %2, Context: %3").arg(sqlInfo).arg(errorMessage).arg(context));
    }
}

void DBLogger::queryExecution(const QString &operation, const QString &sql, bool success, const QString &errorMessage)
{
    if (success) {
        LOG_INFO(LogModule::DATABASE, LogLayer::DATA, operation, 
                 QString("SQL: %1").arg(sql));
    } else {
        LOG_ERROR(LogModule::DATABASE, LogLayer::DATA, operation, 
                  QString("SQL: %1, Error: %2").arg(sql).arg(errorMessage));
    }
}

// 连接相关日志实现
void DBLogger::connectionEstablished(const QString &connectionName)
{
    LOG_INFO(LogModule::DATABASE, LogLayer::DATA, "数据库连接建立", 
             QString("Connection: %1").arg(connectionName));
}

void DBLogger::connectionFailed(const QString &connectionName, const QString &error)
{
    LOG_ERROR(LogModule::DATABASE, LogLayer::DATA, "数据库连接失败", 
              QString("Connection: %1, Error: %2").arg(connectionName).arg(error));
}

void DBLogger::connectionClosed(const QString &connectionName)
{
    LOG_INFO(LogModule::DATABASE, LogLayer::DATA, "数据库连接关闭", 
             QString("Connection: %1").arg(connectionName));
}

// 事务相关日志实现
void DBLogger::transactionBegin(const QString &operation)
{
    LOG_INFO(LogModule::DATABASE, LogLayer::DATA, "事务开始", operation);
}

void DBLogger::transactionCommit(const QString &operation, bool success)
{
    if (success) {
        LOG_INFO(LogModule::DATABASE, LogLayer::DATA, "事务提交成功", operation);
    } else {
        LOG_ERROR(LogModule::DATABASE, LogLayer::DATA, "事务提交失败", operation);
    }
}

void DBLogger::transactionRollback(const QString &operation, const QString &reason)
{
    if (reason.isEmpty()) {
        LOG_WARNING(LogModule::DATABASE, LogLayer::DATA, "事务回滚", operation);
    } else {
        LOG_WARNING(LogModule::DATABASE, LogLayer::DATA, "事务回滚", 
                    QString("%1, Reason: %2").arg(operation).arg(reason));
    }
}

// 数据操作日志实现
void DBLogger::dataInserted(const QString &table, int affectedRows)
{
    LOG_INFO(LogModule::DATABASE, LogLayer::DATA, "数据插入", 
             QString("Table: %1, AffectedRows: %2").arg(table).arg(affectedRows));
}

void DBLogger::dataUpdated(const QString &table, int affectedRows, const QString &condition)
{
    QString message = QString("Table: %1, AffectedRows: %2").arg(table).arg(affectedRows);
    if (!condition.isEmpty()) {
        message += QString(", Condition: %1").arg(condition);
    }
    LOG_INFO(LogModule::DATABASE, LogLayer::DATA, "数据更新", message);
}

void DBLogger::dataDeleted(const QString &table, int affectedRows, const QString &condition)
{
    QString message = QString("Table: %1, AffectedRows: %2").arg(table).arg(affectedRows);
    if (!condition.isEmpty()) {
        message += QString(", Condition: %1").arg(condition);
    }
    LOG_INFO(LogModule::DATABASE, LogLayer::DATA, "数据删除", message);
}

void DBLogger::dataSelected(const QString &table, int resultCount, const QString &condition)
{
    QString message = QString("Table: %1, ResultCount: %2").arg(table).arg(resultCount);
    if (!condition.isEmpty()) {
        message += QString(", Condition: %1").arg(condition);
    }
    LOG_DEBUG(LogModule::DATABASE, LogLayer::DATA, "数据查询", message);
}

// 私有方法实现
QString DBLogger::formatSqlError(const QSqlError &error)
{
    QString errorText = error.text();
    QString driverText = error.driverText();
    QString databaseText = error.databaseText();
    
    QString result = errorText;
    if (!driverText.isEmpty() && driverText != errorText) {
        result += QString(", Driver: %1").arg(driverText);
    }
    if (!databaseText.isEmpty() && databaseText != errorText) {
        result += QString(", Database: %1").arg(databaseText);
    }
    
    return result;
}

QString DBLogger::formatSqlQuery(const QSqlQuery &query)
{
    QString sql = query.lastQuery();
    // 截断过长的SQL语句
    if (sql.length() > 200) {
        sql = sql.left(200) + "...";
    }
    return sql;
}

QString DBLogger::getDatabaseContext(const QSqlQuery &query)
{
    // QSqlQuery 没有 database() 方法，返回默认上下文
    return "Database Context";
}
