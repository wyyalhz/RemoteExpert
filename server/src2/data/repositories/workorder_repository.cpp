#include "workorder_repository.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDateTime>

WorkOrderRepository::WorkOrderRepository(QObject *parent) : DBBase(parent) {}

WorkOrderRepository::~WorkOrderRepository() {}

bool WorkOrderRepository::create(const WorkOrderModel& workOrder, int& workOrderId)
{
    if (!checkConnection("Create work order")) {
        return false;
    }

    // 生成工单号
    QString ticketId = workOrder.ticketId.isEmpty() ? generateTicketId() : workOrder.ticketId;
    
    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO work_orders (ticket_id, title, description, creator_id, priority, category, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )");
    
    query.addBindValue(ticketId);
    query.addBindValue(workOrder.title);
    query.addBindValue(workOrder.description);
    query.addBindValue(workOrder.creatorId);
    query.addBindValue(workOrder.priority);
    query.addBindValue(workOrder.category);

    if (!executeWorkOrderQuery(query, "Create work order")) {
        return false;
    }

    // 获取新创建的工单ID
    workOrderId = query.lastInsertId().toInt();
    
    // 自动将创建者添加为参与者
    if (workOrderId > 0) {
        addParticipant(workOrderId, workOrder.creatorId, ParticipantModel::ROLE_CREATOR);
    }

    logInfo("Create work order", QString("Work order created successfully: %1 ID: %2").arg(ticketId).arg(workOrderId));
    return true;
}

bool WorkOrderRepository::findById(int workOrderId, WorkOrderModel& workOrder)
{
    if (!checkConnection("Find work order by ID")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM work_orders WHERE id = ?");
    query.addBindValue(workOrderId);

    if (!executeWorkOrderQuery(query, "Find work order by ID")) {
        return false;
    }

    if (query.next()) {
        workOrder = mapToModel(query.record());
        return true;
    }

    return false;
}

bool WorkOrderRepository::findByTicketId(const QString& ticketId, WorkOrderModel& workOrder)
{
    if (!checkConnection("Find work order by ticket ID")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM work_orders WHERE ticket_id = ?");
    query.addBindValue(ticketId);

    if (!executeWorkOrderQuery(query, "Find work order by ticket ID")) {
        return false;
    }

    if (query.next()) {
        workOrder = mapToModel(query.record());
        return true;
    }

    return false;
}

bool WorkOrderRepository::update(const WorkOrderModel& workOrder)
{
    if (!checkConnection("Update work order")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE work_orders 
        SET title = ?, description = ?, status = ?, priority = ?, category = ?, 
            assigned_to = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )");
    
    query.addBindValue(workOrder.title);
    query.addBindValue(workOrder.description);
    query.addBindValue(workOrder.status);
    query.addBindValue(workOrder.priority);
    query.addBindValue(workOrder.category);
    query.addBindValue(workOrder.assignedTo);
    query.addBindValue(workOrder.id);

    return executeWorkOrderQuery(query, "Update work order");
}

bool WorkOrderRepository::remove(int workOrderId)
{
    if (!checkConnection("Remove work order")) {
        return false;
    }

    // 先删除参与者
    QSqlQuery participantQuery(database());
    participantQuery.prepare("DELETE FROM work_order_participants WHERE work_order_id = ?");
    participantQuery.addBindValue(workOrderId);
    
    if (!executeParticipantQuery(participantQuery, "Remove work order participants")) {
        return false;
    }

    // 再删除工单
    QSqlQuery query(database());
    query.prepare("DELETE FROM work_orders WHERE id = ?");
    query.addBindValue(workOrderId);

    return executeWorkOrderQuery(query, "Remove work order");
}

QList<WorkOrderModel> WorkOrderRepository::findByStatus(const QString& status)
{
    QList<WorkOrderModel> workOrders;
    
    if (!checkConnection("Find work orders by status")) {
        return workOrders;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM work_orders WHERE status = ? ORDER BY created_at DESC");
    query.addBindValue(status);

    if (!executeWorkOrderQuery(query, "Find work orders by status")) {
        return workOrders;
    }

    while (query.next()) {
        workOrders.append(mapToModel(query.record()));
    }

    return workOrders;
}

QList<WorkOrderModel> WorkOrderRepository::findByCreator(int creatorId)
{
    QList<WorkOrderModel> workOrders;
    
    if (!checkConnection("Find work orders by creator")) {
        return workOrders;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM work_orders WHERE creator_id = ? ORDER BY created_at DESC");
    query.addBindValue(creatorId);

    if (!executeWorkOrderQuery(query, "Find work orders by creator")) {
        return workOrders;
    }

    while (query.next()) {
        workOrders.append(mapToModel(query.record()));
    }

    return workOrders;
}

QList<WorkOrderModel> WorkOrderRepository::findByAssignee(int assigneeId)
{
    QList<WorkOrderModel> workOrders;
    
    if (!checkConnection("Find work orders by assignee")) {
        return workOrders;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM work_orders WHERE assigned_to = ? ORDER BY created_at DESC");
    query.addBindValue(assigneeId);

    if (!executeWorkOrderQuery(query, "Find work orders by assignee")) {
        return workOrders;
    }

    while (query.next()) {
        workOrders.append(mapToModel(query.record()));
    }

    return workOrders;
}

QList<WorkOrderModel> WorkOrderRepository::findByPriority(const QString& priority)
{
    QList<WorkOrderModel> workOrders;
    
    if (!checkConnection("Find work orders by priority")) {
        return workOrders;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM work_orders WHERE priority = ? ORDER BY created_at DESC");
    query.addBindValue(priority);

    if (!executeWorkOrderQuery(query, "Find work orders by priority")) {
        return workOrders;
    }

    while (query.next()) {
        workOrders.append(mapToModel(query.record()));
    }

    return workOrders;
}

QList<WorkOrderModel> WorkOrderRepository::findByCategory(const QString& category)
{
    QList<WorkOrderModel> workOrders;
    
    if (!checkConnection("Find work orders by category")) {
        return workOrders;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM work_orders WHERE category = ? ORDER BY created_at DESC");
    query.addBindValue(category);

    if (!executeWorkOrderQuery(query, "Find work orders by category")) {
        return workOrders;
    }

    while (query.next()) {
        workOrders.append(mapToModel(query.record()));
    }

    return workOrders;
}

QList<WorkOrderModel> WorkOrderRepository::findAll(int limit, int offset)
{
    QList<WorkOrderModel> workOrders;
    
    if (!checkConnection("Find all work orders")) {
        return workOrders;
    }

    QString sql = "SELECT * FROM work_orders ORDER BY created_at DESC";
    if (limit > 0) {
        sql += QString(" LIMIT %1 OFFSET %2").arg(limit).arg(offset);
    }

    QSqlQuery query(database());
    query.prepare(sql);

    if (!executeWorkOrderQuery(query, "Find all work orders")) {
        return workOrders;
    }

    while (query.next()) {
        workOrders.append(mapToModel(query.record()));
    }

    return workOrders;
}

bool WorkOrderRepository::addParticipant(int workOrderId, int userId, const QString& role, const QString& permissions)
{
    if (!checkConnection("Add work order participant")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        INSERT OR REPLACE INTO work_order_participants 
        (work_order_id, user_id, role, permissions, joined_at) 
        VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP)
    )");
    
    query.addBindValue(workOrderId);
    query.addBindValue(userId);
    query.addBindValue(role);
    query.addBindValue(permissions);

    return executeParticipantQuery(query, "Add work order participant");
}

bool WorkOrderRepository::removeParticipant(int workOrderId, int userId)
{
    if (!checkConnection("Remove work order participant")) {
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

    return executeParticipantQuery(query, "Remove work order participant");
}

bool WorkOrderRepository::updateParticipantRole(int workOrderId, int userId, const QString& role)
{
    if (!checkConnection("Update participant role")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE work_order_participants 
        SET role = ? 
        WHERE work_order_id = ? AND user_id = ?
    )");
    
    query.addBindValue(role);
    query.addBindValue(workOrderId);
    query.addBindValue(userId);

    return executeParticipantQuery(query, "Update participant role");
}

QList<ParticipantModel> WorkOrderRepository::getParticipants(int workOrderId)
{
    QList<ParticipantModel> participants;
    
    if (!checkConnection("Get work order participants")) {
        return participants;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        SELECT * FROM work_order_participants 
        WHERE work_order_id = ? AND left_at IS NULL 
        ORDER BY joined_at ASC
    )");
    query.addBindValue(workOrderId);

    if (!executeParticipantQuery(query, "Get work order participants")) {
        return participants;
    }

    while (query.next()) {
        participants.append(mapToParticipantModel(query.record()));
    }

    return participants;
}

bool WorkOrderRepository::isParticipant(int workOrderId, int userId)
{
    if (!checkConnection("Check if user is participant")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        SELECT COUNT(*) FROM work_order_participants 
        WHERE work_order_id = ? AND user_id = ? AND left_at IS NULL
    )");
    query.addBindValue(workOrderId);
    query.addBindValue(userId);

    if (!executeParticipantQuery(query, "Check if user is participant")) {
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

bool WorkOrderRepository::updateStatus(int workOrderId, const QString& status)
{
    if (!checkConnection("Update work order status")) {
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

    return executeWorkOrderQuery(query, "Update work order status");
}

bool WorkOrderRepository::assignTo(int workOrderId, int expertId)
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

    return executeWorkOrderQuery(query, "Assign work order");
}

bool WorkOrderRepository::closeWorkOrder(int workOrderId)
{
    if (!checkConnection("Close work order")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE work_orders 
        SET status = ?, closed_at = CURRENT_TIMESTAMP, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    query.addBindValue(WorkOrderModel::STATUS_CLOSED);
    query.addBindValue(workOrderId);

    return executeWorkOrderQuery(query, "Close work order");
}

int WorkOrderRepository::countByStatus(const QString& status)
{
    if (!checkConnection("Count work orders by status")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM work_orders WHERE status = ?");
    query.addBindValue(status);

    if (!executeWorkOrderQuery(query, "Count work orders by status")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int WorkOrderRepository::countByCreator(int creatorId)
{
    if (!checkConnection("Count work orders by creator")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM work_orders WHERE creator_id = ?");
    query.addBindValue(creatorId);

    if (!executeWorkOrderQuery(query, "Count work orders by creator")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int WorkOrderRepository::countByAssignee(int assigneeId)
{
    if (!checkConnection("Count work orders by assignee")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM work_orders WHERE assigned_to = ?");
    query.addBindValue(assigneeId);

    if (!executeWorkOrderQuery(query, "Count work orders by assignee")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int WorkOrderRepository::countAll()
{
    if (!checkConnection("Count all work orders")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM work_orders");

    if (!executeWorkOrderQuery(query, "Count all work orders")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

QString WorkOrderRepository::generateTicketId()
{
    if (!checkConnection("Generate ticket ID")) {
        return QString();
    }

    QSqlQuery query(database());
    query.prepare("SELECT MAX(CAST(ticket_id AS INTEGER)) FROM work_orders");
    
    if (!executeWorkOrderQuery(query, "Query max work order ID")) {
        return QString();
    }
    
    int maxId = 0;
    if (query.next()) {
        maxId = query.value(0).toInt();
    }
    
    // 生成新的工单号（最大号+1）
    return QString::number(maxId + 1);
}

WorkOrderModel WorkOrderRepository::mapToModel(const QSqlRecord& record)
{
    WorkOrderModel model;
    model.id = record.value("id").toInt();
    model.ticketId = record.value("ticket_id").toString();
    model.title = record.value("title").toString();
    model.description = record.value("description").toString();
    model.creatorId = record.value("creator_id").toInt();
    model.status = record.value("status").toString();
    model.priority = record.value("priority").toString();
    model.category = record.value("category").toString();
    model.assignedTo = record.value("assigned_to").toInt();
    model.createdAt = record.value("created_at").toDateTime();
    model.updatedAt = record.value("updated_at").toDateTime();
    
    if (!record.value("closed_at").isNull()) {
        model.closedAt = record.value("closed_at").toDateTime();
    }
    
    return model;
}

ParticipantModel WorkOrderRepository::mapToParticipantModel(const QSqlRecord& record)
{
    ParticipantModel model;
    model.id = record.value("id").toInt();
    model.workOrderId = record.value("work_order_id").toInt();
    model.userId = record.value("user_id").toInt();
    model.role = record.value("role").toString();
    model.joinedAt = record.value("joined_at").toDateTime();
    model.permissions = record.value("permissions").toString();
    
    if (!record.value("left_at").isNull()) {
        model.leftAt = record.value("left_at").toDateTime();
    }
    
    return model;
}

bool WorkOrderRepository::executeWorkOrderQuery(QSqlQuery& query, const QString& operation)
{
    return executeQuery(query, operation);
}

bool WorkOrderRepository::executeParticipantQuery(QSqlQuery& query, const QString& operation)
{
    return executeQuery(query, operation);
}
