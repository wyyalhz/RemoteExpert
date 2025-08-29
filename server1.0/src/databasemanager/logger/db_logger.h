#ifndef DB_LOGGER_H
#define DB_LOGGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QJsonObject>

class DBLogger : public QObject
{
    Q_OBJECT
public:
    explicit DBLogger(QObject *parent = nullptr);
    ~DBLogger();

    // 设置数据库连接
    void setDatabase(QSqlDatabase &db) { db_ = db; }
    
    // 日志记录
    void logInfo(const QString &module, const QString &message, const QJsonObject &context = QJsonObject());
    void logWarning(const QString &module, const QString &message, const QJsonObject &context = QJsonObject());
    void logError(const QString &module, const QString &message, const QJsonObject &context = QJsonObject());
    void logDebug(const QString &module, const QString &message, const QJsonObject &context = QJsonObject());
    
    // 工单操作日志
    void logWorkOrderCreated(int workOrderId, int userId, const QString &ticketId);
    void logWorkOrderJoined(int workOrderId, int userId, const QString &role);
    void logWorkOrderLeft(int workOrderId, int userId);
    void logWorkOrderStatusChanged(int workOrderId, int userId, const QString &oldStatus, const QString &newStatus);
    void logWorkOrderClosed(int workOrderId, int userId);
    void logWorkOrderAssigned(int workOrderId, int expertId, int assignedBy);

private:
    QSqlDatabase db_;
    
    // 创建日志表
    bool createLogTable();
    
    // 通用日志记录
    void logMessage(const QString &level, const QString &module, const QString &message, const QJsonObject &context);
};

#endif // DB_LOGGER_H
