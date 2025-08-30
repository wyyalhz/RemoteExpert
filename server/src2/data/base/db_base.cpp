#include "db_base.h"

DBBase::DBBase(QObject *parent) : QObject(parent) {}

DBBase::~DBBase() {}

bool DBBase::checkConnection(const QString &operation)
{
    if (!db_.isOpen()) {
        lastError_ = QString("Database connection is not open for operation: %1").arg(operation);
        logError(operation, QSqlError(lastError_, "", QSqlError::ConnectionError));
        return false;
    }
    return true;
}

void DBBase::logError(const QString &operation, const QSqlError &error)
{
    lastError_ = error.text();
    qCritical() << "Database error in" << operation << ":" << error.text();
}

void DBBase::logWarning(const QString &operation, const QString &message)
{
    qWarning() << "Database warning in" << operation << ":" << message;
}

void DBBase::logInfo(const QString &operation, const QString &message)
{
    qInfo() << "Database info in" << operation << ":" << message;
}

bool DBBase::executeQuery(QSqlQuery &query, const QString &operation)
{
    if (!checkConnection(operation)) {
        return false;
    }
    
    if (!query.exec()) {
        logError(operation, query.lastError());
        return false;
    }
    
    logInfo(operation, "Query executed successfully");
    return true;
}

bool DBBase::checkQueryResult(QSqlQuery &query, const QString &operation)
{
    if (query.lastError().isValid()) {
        logError(operation, query.lastError());
        return false;
    }
    return true;
}

QJsonObject DBBase::getQueryResultAsJson(QSqlQuery &query)
{
    QJsonObject result;
    QSqlRecord record = query.record();
    
    for (int i = 0; i < record.count(); ++i) {
        QString fieldName = record.fieldName(i);
        QVariant value = record.value(i);
        
        if (value.isNull()) {
            result[fieldName] = QJsonValue::Null;
        } else {
            switch (value.type()) {
                case QVariant::Int:
                    result[fieldName] = value.toInt();
                    break;
                case QVariant::Double:
                    result[fieldName] = value.toDouble();
                    break;
                case QVariant::Bool:
                    result[fieldName] = value.toBool();
                    break;
                case QVariant::DateTime:
                    result[fieldName] = value.toDateTime().toString(Qt::ISODate);
                    break;
                default:
                    result[fieldName] = value.toString();
                    break;
            }
        }
    }
    
    return result;
}

QJsonArray DBBase::getQueryResultsAsJsonArray(QSqlQuery &query)
{
    QJsonArray results;
    
    while (query.next()) {
        results.append(getQueryResultAsJson(query));
    }
    
    return results;
}

QList<QSqlRecord> DBBase::paginate(QSqlQuery &query, int page, int pageSize)
{
    QList<QSqlRecord> records;
    int offset = page * pageSize;
    
    // 设置分页参数（如果查询支持）
    if (query.isSelect()) {
        // 注意：SQLite的LIMIT和OFFSET需要手动添加
        // 这里假设查询已经包含了LIMIT和OFFSET
    }
    
    while (query.next()) {
        records.append(query.record());
    }
    
    return records;
}
