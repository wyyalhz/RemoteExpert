#include "workorder_manager.h"

WorkOrderManager::WorkOrderManager(QObject *parent) : DBBase(parent) {}

WorkOrderManager::~WorkOrderManager() {}

bool WorkOrderManager::createWorkOrder(const QString &title, const QString &description,
                                     int creatorId, const QString &priority, const QString &category, QString &generatedTicketId)
{
    if (!checkConnection("Create work order")) {
        return false;
    }

    // 自动生成工单号
    QSqlQuery query(database());
    query.prepare("SELECT MAX(CAST(ticket_id AS INTEGER)) FROM work_orders");
    
    if (!executeQuery(query, "Query max work order ID")) {
        return false;
    }
    
    int maxId = 0;
    if (query.next()) {
        maxId = query.value(0).toInt();
    }
    
    // 生成新的工单号（最大号+1）
    generatedTicketId = QString::number(maxId + 1);
    
    logInfo("Create work order", QString("Generated work order ID: %1").arg(generatedTicketId));

    // 插入工单记录
    query.prepare(R"(
        INSERT INTO work_orders (ticket_id, title, description, creator_id, priority, category, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )");
    
    query.addBindValue(generatedTicketId);
    query.addBindValue(title);
    query.addBindValue(description);
    query.addBindValue(creatorId);
    query.addBindValue(priority);
    query.addBindValue(category);

    if (!executeQuery(query, "Create work order")) {
        return false;
    }

    // 获取新创建的工单ID
    int workOrderId = query.lastInsertId().toInt();
    
    // 自动将创建者添加为参与者
    if (workOrderId > 0) {
        joinWorkOrder(workOrderId, creatorId, "creator");
    }

    logInfo("Create work order", QString("Work order created successfully: %1 ID: %2").arg(generatedTicketId).arg(workOrderId));
    return true;
}

bool WorkOrderManager::joinWorkOrder(int workOrderId, int userId, const QString &role)
{
    if (!checkConnection("Join work order")) {
        return false;
    }

    QSqlQuery query(database());
    
    // 检查是否已经参与该工单
    query.prepare("SELECT id FROM work_order_participants WHERE work_order_id = ? AND user_id = ?");
    query.addBindValue(workOrderId);
    query.addBindValue(userId);
    
    if (!executeQuery(query, "Check participant")) {
        return false;
    }

    if (query.next()) {
        // 如果已存在，更新角色和加入时间
        query.prepare(R"(
            UPDATE work_order_participants 
            SET role = ?, joined_at = CURRENT_TIMESTAMP, left_at = NULL 
            WHERE work_order_id = ? AND user_id = ?
        )");
        query.addBindValue(role);
        query.addBindValue(workOrderId);
        query.addBindValue(userId);
    } else {
        // 如果不存在，插入新记录
        query.prepare(R"(
            INSERT INTO work_order_participants (work_order_id, user_id, role, joined_at)
            VALUES (?, ?, ?, CURRENT_TIMESTAMP)
        )");
        query.addBindValue(workOrderId);
        query.addBindValue(userId);
        query.addBindValue(role);
    }

    if (!executeQuery(query, "Join work order")) {
        return false;
    }

    logInfo("Join work order", QString("User %1 joined work order %2 as %3").arg(userId).arg(workOrderId).arg(role));
    return true;
}

bool WorkOrderManager::leaveWorkOrder(int workOrderId, int userId)
{
    if (!checkConnection("Leave work order")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE work_order_participants 
        SET left_at = CURRENT_TIMESTAMP 
        WHERE work_order_id = ? AND user_id = ?
    )");
    
    query.addBindValue(workOrderId);
    query.addBindValue(userId);

    if (!executeQuery(query, "Leave work order")) {
        return false;
    }

    logInfo("Leave work order", QString("User %1 left work order %2").arg(userId).arg(workOrderId));
    return true;
}

QJsonArray WorkOrderManager::getWorkOrderParticipants(int workOrderId)
{
    QJsonArray participants;
    
    if (!checkConnection("Get participants")) {
        return participants;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        SELECT wp.user_id, wp.role, wp.joined_at, wp.left_at, u.username
        FROM work_order_participants wp
        JOIN users u ON wp.user_id = u.id
        WHERE wp.work_order_id = ? AND wp.left_at IS NULL
        ORDER BY wp.joined_at
    )");
    
    query.addBindValue(workOrderId);

    if (!executeQuery(query, "Get participants")) {
        return participants;
    }

    participants = getQueryResultsAsJsonArray(query);
    return participants;
}

bool WorkOrderManager::updateWorkOrderStatus(int workOrderId, const QString &status)
{
    if (!checkConnection("Update status")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE work_orders 
        SET status = ?, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    query.addBindValue(status);
    query.addBindValue(workOrderId);

    if (!executeQuery(query, "Update status")) {
        return false;
    }

    logInfo("Update status", QString("Work order %1 status updated to %2").arg(workOrderId).arg(status));
    return true;
}

bool WorkOrderManager::closeWorkOrder(int workOrderId, int userId)
{
    if (!checkConnection("Close work order")) {
        return false;
    }

    // 检查用户是否为工单创建者
    QSqlQuery checkQuery(database());
    checkQuery.prepare("SELECT creator_id FROM work_orders WHERE id = ?");
    checkQuery.addBindValue(workOrderId);
    
    if (!executeQuery(checkQuery, "Check work order creator") || !checkQueryResult(checkQuery, "Work order not found")) {
        return false;
    }
    
    int creatorId = checkQuery.value("creator_id").toInt();
    if (creatorId != userId) {
        logWarning("Close work order", QString("User %1 is not the creator of work order %2").arg(userId).arg(workOrderId));
        return false;
    }

    // 更新工单状态为closed
    QSqlQuery updateQuery(database());
    updateQuery.prepare(R"(
        UPDATE work_orders 
        SET status = 'closed', closed_at = CURRENT_TIMESTAMP, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    updateQuery.addBindValue(workOrderId);

    if (!executeQuery(updateQuery, "Close work order")) {
        return false;
    }

    logInfo("Close work order", QString("Work order %1 status updated to closed by user %2").arg(workOrderId).arg(userId));
    return true;
}

bool WorkOrderManager::assignWorkOrder(int workOrderId, int expertId)
{
    if (!checkConnection("Assign work order")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE work_orders 
        SET assigned_to = ?, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    query.addBindValue(expertId);
    query.addBindValue(workOrderId);

    if (!executeQuery(query, "Assign work order")) {
        return false;
    }

    logInfo("Assign work order", QString("Work order %1 assigned to expert %2").arg(workOrderId).arg(expertId));
    return true;
}

QJsonObject WorkOrderManager::getWorkOrderInfo(int workOrderId)
{
    QJsonObject workOrder;
    
    if (!checkConnection("Get work order info")) {
        return workOrder;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        SELECT wo.*, u.username as creator_name, e.username as assigned_expert_name
        FROM work_orders wo
        JOIN users u ON wo.creator_id = u.id
        LEFT JOIN users e ON wo.assigned_to = e.id
        WHERE wo.id = ?
    )");
    
    query.addBindValue(workOrderId);

    if (!executeQuery(query, "Get work order info")) {
        return workOrder;
    }

    if (query.next()) {
        workOrder = getQueryResultAsJson(query);
    }

    return workOrder;
}
