#include "db_logger.h"
#include <QJsonDocument>

DBLogger::DBLogger(QObject *parent) : DBBase(parent) {}

DBLogger::~DBLogger() {}

void DBLogger::logInfo(const QString &module, const QString &message, const QJsonObject &context)
{
    logMessage("INFO", module, message, context);
}

void DBLogger::logWarning(const QString &module, const QString &message, const QJsonObject &context)
{
    logMessage("WARNING", module, message, context);
}

void DBLogger::logError(const QString &module, const QString &message, const QJsonObject &context)
{
    logMessage("ERROR", module, message, context);
}

void DBLogger::logDebug(const QString &module, const QString &message, const QJsonObject &context)
{
    logMessage("DEBUG", module, message, context);
}

void DBLogger::logWorkOrderCreated(int workOrderId, int userId, const QString &ticketId)
{
    QJsonObject context;
    context["work_order_id"] = workOrderId;
    context["user_id"] = userId;
    context["ticket_id"] = ticketId;
    
    logInfo("WorkOrder", QString("Work order created: %1").arg(ticketId), context);
}

void DBLogger::logWorkOrderJoined(int workOrderId, int userId, const QString &role)
{
    QJsonObject context;
    context["work_order_id"] = workOrderId;
    context["user_id"] = userId;
    context["role"] = role;
    
    logInfo("WorkOrder", QString("User %1 joined work order as %2").arg(userId).arg(role), context);
}

void DBLogger::logWorkOrderLeft(int workOrderId, int userId)
{
    QJsonObject context;
    context["work_order_id"] = workOrderId;
    context["user_id"] = userId;
    
    logInfo("WorkOrder", QString("User %1 left work order").arg(userId), context);
}

void DBLogger::logWorkOrderStatusChanged(int workOrderId, int userId, const QString &oldStatus, const QString &newStatus)
{
    QJsonObject context;
    context["work_order_id"] = workOrderId;
    context["user_id"] = userId;
    context["old_status"] = oldStatus;
    context["new_status"] = newStatus;
    
    logInfo("WorkOrder", QString("Status changed from %1 to %2").arg(oldStatus).arg(newStatus), context);
}

void DBLogger::logWorkOrderClosed(int workOrderId, int userId)
{
    QJsonObject context;
    context["work_order_id"] = workOrderId;
    context["user_id"] = userId;
    
    logInfo("WorkOrder", QString("Work order closed by user %1").arg(userId), context);
}

void DBLogger::logWorkOrderAssigned(int workOrderId, int expertId, int assignedBy)
{
    QJsonObject context;
    context["work_order_id"] = workOrderId;
    context["expert_id"] = expertId;
    context["assigned_by"] = assignedBy;
    
    logInfo("WorkOrder", QString("Work order assigned to expert %1 by user %2").arg(expertId).arg(assignedBy), context);
}

bool DBLogger::createLogTable()
{
    if (!checkConnection("Create log table")) {
        return false;
    }

    QSqlQuery query(database());
    QString createLogTable = R"(
        CREATE TABLE IF NOT EXISTS system_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            level TEXT NOT NULL,
            module TEXT NOT NULL,
            message TEXT NOT NULL,
            context TEXT,
            user_id INTEGER,
            work_order_id INTEGER,
            FOREIGN KEY (user_id) REFERENCES users (id),
            FOREIGN KEY (work_order_id) REFERENCES work_orders (id)
        )
    )";

    if (!executeQuery(query, "Create system_logs table")) {
        return false;
    }

    return true;
}

void DBLogger::logMessage(const QString &level, const QString &module, const QString &message, const QJsonObject &context)
{
    if (!checkConnection("Log message")) {
        // 如果数据库不可用，使用控制台日志
        qDebug() << QString("[%1] %2: %3").arg(level).arg(module).arg(message);
        return;
    }

    // 确保日志表存在
    if (!createLogTable()) {
        return;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO system_logs (level, module, message, context, timestamp)
        VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP)
    )");
    
    query.addBindValue(level);
    query.addBindValue(module);
    query.addBindValue(message);
    
    QString contextJson = QJsonDocument(context).toJson(QJsonDocument::Compact);
    query.addBindValue(contextJson);

    if (!executeQuery(query, "Insert log message")) {
        // 如果数据库插入失败，使用控制台日志作为备选
        qDebug() << QString("[%1] %2: %3").arg(level).arg(module).arg(message);
    }
}
