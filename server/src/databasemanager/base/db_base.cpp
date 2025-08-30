#include "db_base.h"

DBBase::DBBase(QObject *parent) : QObject(parent) {}

DBBase::~DBBase() {}

bool DBBase::checkConnection(const QString &operation)
{
    if (!db_.isOpen()) {
        qCritical() << QString("[%1] %2 failed: Database is NOT open!").arg(metaObject()->className()).arg(operation);
        return false;
    }
    return true;
}

void DBBase::logError(const QString &operation, const QSqlError &error)
{
    qCritical() << QString("[%1] %2 failed: %3").arg(metaObject()->className()).arg(operation).arg(error.text());
}

void DBBase::logWarning(const QString &operation, const QString &message)
{
    qWarning() << QString("[%1] %2: %3").arg(metaObject()->className()).arg(operation).arg(message);
}

void DBBase::logInfo(const QString &operation, const QString &message)
{
    qInfo() << QString("[%1] %2: %3").arg(metaObject()->className()).arg(operation).arg(message);
}

bool DBBase::executeQuery(QSqlQuery &query, const QString &operation)
{
    if (!query.exec()) {
        logError(operation, query.lastError());
        return false;
    }
    return true;
}

bool DBBase::checkQueryResult(QSqlQuery &query, const QString &operation)
{
    if (!query.next()) {
        logWarning(operation, "No results found");
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
        QVariant value = query.value(i);
        
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
        default:
            result[fieldName] = value.toString();
            break;
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
