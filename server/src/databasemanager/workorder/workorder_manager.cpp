#include "workorder_manager.h"

WorkOrderManager::WorkOrderManager(QObject *parent) : QObject(parent) {}

WorkOrderManager::~WorkOrderManager() {}

bool WorkOrderManager::createWorkOrder(const QString &title, const QString &description,
                                     int creatorId, const QString &priority, const QString &category, QString &generatedTicketId)
{
    if (!db_.isOpen()) {
        qCritical() << "[WorkOrderManager] Create work order failed: Database is NOT open!";
        return false;
    }

    // 自动生成工单号
    QSqlQuery query(db_);
    query.prepare("SELECT MAX(CAST(ticket_id AS INTEGER)) FROM work_orders");
    
    if (!query.exec()) {
        qCritical() << "[WorkOrderManager] Query max work order ID failed:" << query.lastError().text();
        return false;
    }
    
    int maxId = 0;
    if (query.next()) {
        maxId = query.value(0).toInt();
    }
    
    // 生成新的工单号（最大号+1）
    generatedTicketId = QString::number(maxId + 1);
    
    qInfo() << "[WorkOrderManager] Generated work order ID:" << generatedTicketId;

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

    if (!query.exec()) {
        qCritical() << "[WorkOrderManager] Create work order failed:" << query.lastError().text();
        return false;
    }

    // 获取新创建的工单ID
    int workOrderId = query.lastInsertId().toInt();
    
    // 自动将创建者添加为参与者
    if (workOrderId > 0) {
        joinWorkOrder(workOrderId, creatorId, "creator");
    }

    qInfo() << "[WorkOrderManager] Work order created successfully:" << generatedTicketId << "ID:" << workOrderId;
    return true;
}

bool WorkOrderManager::joinWorkOrder(int workOrderId, int userId, const QString &role)
{
    if (!db_.isOpen()) {
        qCritical() << "[WorkOrderManager] Join work order failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    
    // 检查是否已经参与该工单
    query.prepare("SELECT id FROM work_order_participants WHERE work_order_id = ? AND user_id = ?");
    query.addBindValue(workOrderId);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qCritical() << "[WorkOrderManager] Check participant failed:" << query.lastError().text();
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

    if (!query.exec()) {
        qCritical() << "[WorkOrderManager] Join work order failed:" << query.lastError().text();
        return false;
    }

    qInfo() << "[WorkOrderManager] User" << userId << "joined work order" << workOrderId << "as" << role;
    return true;
}

bool WorkOrderManager::leaveWorkOrder(int workOrderId, int userId)
{
    if (!db_.isOpen()) {
        qCritical() << "[WorkOrderManager] Leave work order failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        UPDATE work_order_participants 
        SET left_at = CURRENT_TIMESTAMP 
        WHERE work_order_id = ? AND user_id = ?
    )");
    
    query.addBindValue(workOrderId);
    query.addBindValue(userId);

    if (!query.exec()) {
        qCritical() << "[WorkOrderManager] Leave work order failed:" << query.lastError().text();
        return false;
    }

    qInfo() << "[WorkOrderManager] User" << userId << "left work order" << workOrderId;
    return true;
}

QJsonArray WorkOrderManager::getWorkOrderParticipants(int workOrderId)
{
    QJsonArray participants;
    
    if (!db_.isOpen()) {
        qCritical() << "[WorkOrderManager] Get participants failed: Database is NOT open!";
        return participants;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        SELECT wp.user_id, wp.role, wp.joined_at, wp.left_at, u.username
        FROM work_order_participants wp
        JOIN users u ON wp.user_id = u.id
        WHERE wp.work_order_id = ? AND wp.left_at IS NULL
        ORDER BY wp.joined_at
    )");
    
    query.addBindValue(workOrderId);

    if (!query.exec()) {
        qCritical() << "[WorkOrderManager] Get participants failed:" << query.lastError().text();
        return participants;
    }

    while (query.next()) {
        QJsonObject participant;
        participant["user_id"] = query.value("user_id").toInt();
        participant["username"] = query.value("username").toString();
        participant["role"] = query.value("role").toString();
        participant["joined_at"] = query.value("joined_at").toString();
        participants.append(participant);
    }

    return participants;
}

bool WorkOrderManager::updateWorkOrderStatus(int workOrderId, const QString &status)
{
    if (!db_.isOpen()) {
        qCritical() << "[WorkOrderManager] Update status failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        UPDATE work_orders 
        SET status = ?, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    query.addBindValue(status);
    query.addBindValue(workOrderId);

    if (!query.exec()) {
        qCritical() << "[WorkOrderManager] Update status failed:" << query.lastError().text();
        return false;
    }

    qInfo() << "[WorkOrderManager] Work order" << workOrderId << "status updated to" << status;
    return true;
}

bool WorkOrderManager::closeWorkOrder(int workOrderId, int userId)
{
    if (!db_.isOpen()) {
        qCritical() << "[WorkOrderManager] Close work order failed: Database is NOT open!";
        return false;
    }

    // 检查用户是否为工单创建者
    QSqlQuery checkQuery(db_);
    checkQuery.prepare("SELECT creator_id FROM work_orders WHERE id = ?");
    checkQuery.addBindValue(workOrderId);
    
    if (!checkQuery.exec() || !checkQuery.next()) {
        qCritical() << "[WorkOrderManager] Work order not found:" << workOrderId;
        return false;
    }
    
    int creatorId = checkQuery.value("creator_id").toInt();
    if (creatorId != userId) {
        qWarning() << "[WorkOrderManager] User" << userId << "is not the creator of work order" << workOrderId;
        return false;
    }

    // 更新工单状态为closed
    QSqlQuery updateQuery(db_);
    updateQuery.prepare(R"(
        UPDATE work_orders 
        SET status = 'closed', closed_at = CURRENT_TIMESTAMP, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    updateQuery.addBindValue(workOrderId);

    if (!updateQuery.exec()) {
        qCritical() << "[WorkOrderManager] Close work order failed:" << updateQuery.lastError().text();
        return false;
    }

    qInfo() << "[WorkOrderManager] Work order" << workOrderId << "status updated to closed by user" << userId;
    return true;
}

bool WorkOrderManager::assignWorkOrder(int workOrderId, int expertId)
{
    if (!db_.isOpen()) {
        qCritical() << "[WorkOrderManager] Assign work order failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        UPDATE work_orders 
        SET assigned_to = ?, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    query.addBindValue(expertId);
    query.addBindValue(workOrderId);

    if (!query.exec()) {
        qCritical() << "[WorkOrderManager] Assign work order failed:" << query.lastError().text();
        return false;
    }

    qInfo() << "[WorkOrderManager] Work order" << workOrderId << "assigned to expert" << expertId;
    return true;
}

QJsonObject WorkOrderManager::getWorkOrderInfo(int workOrderId)
{
    QJsonObject workOrder;
    
    if (!db_.isOpen()) {
        qCritical() << "[WorkOrderManager] Get work order info failed: Database is NOT open!";
        return workOrder;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        SELECT wo.*, u.username as creator_name, e.username as assigned_expert_name
        FROM work_orders wo
        JOIN users u ON wo.creator_id = u.id
        LEFT JOIN users e ON wo.assigned_to = e.id
        WHERE wo.id = ?
    )");
    
    query.addBindValue(workOrderId);

    if (!query.exec()) {
        qCritical() << "[WorkOrderManager] Get work order info failed:" << query.lastError().text();
        return workOrder;
    }

    if (query.next()) {
        workOrder["id"] = query.value("id").toInt();
        workOrder["ticket_id"] = query.value("ticket_id").toString();
        workOrder["title"] = query.value("title").toString();
        workOrder["description"] = query.value("description").toString();
        workOrder["status"] = query.value("status").toString();
        workOrder["priority"] = query.value("priority").toString();
        workOrder["category"] = query.value("category").toString();
        workOrder["creator_id"] = query.value("creator_id").toInt();
        workOrder["creator_name"] = query.value("creator_name").toString();
        workOrder["assigned_to"] = query.value("assigned_to").toInt();
        workOrder["assigned_expert_name"] = query.value("assigned_expert_name").toString();
        workOrder["created_at"] = query.value("created_at").toString();
        workOrder["updated_at"] = query.value("updated_at").toString();
        workOrder["closed_at"] = query.value("closed_at").toString();
    }

    return workOrder;
}
