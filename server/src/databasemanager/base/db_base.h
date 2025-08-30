#ifndef DB_BASE_H
#define DB_BASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

class DBBase : public QObject
{
    Q_OBJECT
public:
    explicit DBBase(QObject *parent = nullptr);
    virtual ~DBBase();

    // 设置数据库连接
    void setDatabase(QSqlDatabase &db) { db_ = db; }
    
    // 获取数据库连接
    QSqlDatabase& database() { return db_; }
    const QSqlDatabase& database() const { return db_; }

protected:
    // 数据库连接检查
    bool checkConnection(const QString &operation = "Database operation");
    
    // 通用错误处理
    void logError(const QString &operation, const QSqlError &error);
    void logWarning(const QString &operation, const QString &message);
    void logInfo(const QString &operation, const QString &message);
    
    // 通用查询执行
    bool executeQuery(QSqlQuery &query, const QString &operation);
    
    // 检查查询结果
    bool checkQueryResult(QSqlQuery &query, const QString &operation);
    
    // 获取查询结果
    QJsonObject getQueryResultAsJson(QSqlQuery &query);
    QJsonArray getQueryResultsAsJsonArray(QSqlQuery &query);

private:
    QSqlDatabase db_;
};

#endif // DB_BASE_H
