#include "workorder_service.h"
#include "../../../common/protocol/types/enums.h"
#include "../../data/models/user_model.h"

WorkOrderService::WorkOrderService(DatabaseManager* dbManager, UserService* userService, QObject *parent)
    : QObject(parent), dbManager_(dbManager), workOrderRepo_(dbManager->workOrderRepository()), userService_(userService)
{
    BusinessLogger::info("Work Order Service", "Work order service initialized");
}

WorkOrderService::~WorkOrderService()
{
    BusinessLogger::info("Work Order Service", "Work order service destroyed");
}

// 工单创建和管理
bool WorkOrderService::createWorkOrder(const QString& title, const QString& description, 
                                     int creatorId, const QString& priority, const QString& category, 
                                     const QString& expertUsername, QString& generatedTicketId)
{
    BusinessLogger::businessOperationStart("Work Order Creation", QString("Creator: %1, Expert: %2").arg(creatorId).arg(expertUsername));
    
    try {
        // 数据验证
        WorkOrderValidator::validateCreation(title, description, creatorId, priority, category);
        
        // 根据专家用户名查找专家ID
        BusinessLogger::info("Work Order Service", QString("Looking up expert by username: %1").arg(expertUsername));
        UserModel expert = userService_->getUserInfo(expertUsername);
        
        if (!expert.isValid()) {
            BusinessLogger::error("Work Order Service", QString("Expert user not found: %1").arg(expertUsername));
            BusinessLogger::workOrderCreated("", creatorId, false, QString("Expert not found: %1").arg(expertUsername));
            BusinessLogger::businessOperationFailed("Work Order Creation", QString("Expert not found: %1").arg(expertUsername));
            return false;
        }
        
        if (expert.userType != USER_TYPE_EXPERT) {
            BusinessLogger::error("Work Order Service", QString("User %1 is not an expert (userType: %2)").arg(expertUsername).arg(expert.userType));
            BusinessLogger::workOrderCreated("", creatorId, false, QString("User %1 is not an expert").arg(expertUsername));
            BusinessLogger::businessOperationFailed("Work Order Creation", QString("User %1 is not an expert").arg(expertUsername));
            return false;
        }
        
        BusinessLogger::info("Work Order Service", QString("Expert found: ID=%1, Username=%2, UserType=%3").arg(expert.id).arg(expert.username).arg(expert.userType));
        
        // 生成工单号
        generatedTicketId = generateTicketId();
        BusinessLogger::info("Work Order Service", QString("Generated ticket ID: %1").arg(generatedTicketId));
        
        // 创建工单模型
        WorkOrderModel workOrder;
        workOrder.ticketId = generatedTicketId;
        workOrder.title = title;
        workOrder.description = description;
        workOrder.creatorId = creatorId;
        workOrder.priority = priority;
        workOrder.category = category;
        workOrder.status = WorkOrderStatusManager::OPEN; // 初始状态为待处理
        workOrder.assignedTo = expert.id; // 设置指派专家
        
        BusinessLogger::info("Work Order Service", QString("Work order model prepared - AssignedTo: %1, Status: %2").arg(workOrder.assignedTo).arg(workOrder.status));
        
        // 保存工单
        int workOrderId = -1;
        bool success = workOrderRepo_->create(workOrder, workOrderId);
        
        if (success) {
            // 自动将创建者添加为参与者
            workOrderRepo_->addParticipant(workOrderId, creatorId, ParticipantModel::ROLE_CREATOR);
            
            BusinessLogger::workOrderCreated(generatedTicketId, creatorId, true);
            BusinessLogger::businessOperationSuccess("Work Order Creation", QString("Work order ID: %1, Ticket ID: %2, Expert: %3").arg(workOrderId).arg(generatedTicketId).arg(expertUsername));
        } else {
            BusinessLogger::workOrderCreated(generatedTicketId, creatorId, false, "Database operation failed");
            BusinessLogger::businessOperationFailed("Work Order Creation", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::workOrderCreated("", creatorId, false, e.getMessage());
        BusinessLogger::businessOperationFailed("Work Order Creation", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::workOrderCreated("", creatorId, false, e.getMessage());
        BusinessLogger::businessOperationFailed("Work Order Creation", e.getMessage());
        return false;
    }
}

bool WorkOrderService::updateWorkOrder(const WorkOrderModel& workOrder)
{
    BusinessLogger::businessOperationStart("Work Order Update", workOrder.ticketId);
    
    try {
        // 数据验证
        WorkOrderValidator::validateUpdate(workOrder);
        
        // 检查工单是否存在
        if (!validateWorkOrderExists(workOrder.id)) {
            BusinessLogger::businessOperationFailed("Work Order Update", "Work order not found");
            return false;
        }
        
        // 更新工单
        bool success = workOrderRepo_->update(workOrder);
        
        if (success) {
            BusinessLogger::workOrderUpdated(workOrder.ticketId, "Work order updated", true);
            BusinessLogger::businessOperationSuccess("Work Order Update", workOrder.ticketId);
        } else {
            BusinessLogger::workOrderUpdated(workOrder.ticketId, "", false, "Database operation failed");
            BusinessLogger::businessOperationFailed("Work Order Update", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::workOrderUpdated(workOrder.ticketId, "", false, e.getMessage());
        BusinessLogger::businessOperationFailed("Work Order Update", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::workOrderUpdated(workOrder.ticketId, "", false, e.getMessage());
        BusinessLogger::businessOperationFailed("Work Order Update", e.getMessage());
        return false;
    }
}

bool WorkOrderService::deleteWorkOrder(int workOrderId, int userId)
{
    BusinessLogger::businessOperationStart("Work Order Deletion", QString::number(workOrderId));
    
    try {
        // 检查工单是否存在
        if (!validateWorkOrderExists(workOrderId)) {
            BusinessLogger::businessOperationFailed("Work Order Deletion", "Work order not found");
            return false;
        }
        
        // 检查用户权限
        if (!validateUserPermissions(workOrderId, userId, "delete")) {
            BusinessLogger::businessOperationFailed("Work Order Deletion", "User does not have permission to delete work order");
            return false;
        }
        
        // 删除工单
        bool success = workOrderRepo_->remove(workOrderId);
        
        if (success) {
            BusinessLogger::businessOperationSuccess("Work Order Deletion", QString::number(workOrderId));
        } else {
            BusinessLogger::businessOperationFailed("Work Order Deletion", "Database operation failed");
        }
        
        return success;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Work Order Deletion", e.getMessage());
        return false;
    }
}

// 工单查询
WorkOrderModel WorkOrderService::getWorkOrderById(int workOrderId)
{
    BusinessLogger::businessOperationStart("Get Work Order By ID", QString::number(workOrderId));
    
    try {
        WorkOrderModel workOrder;
        if (workOrderRepo_->findById(workOrderId, workOrder)) {
            BusinessLogger::businessOperationSuccess("Get Work Order By ID", QString::number(workOrderId));
            return workOrder;
        } else {
            BusinessLogger::businessOperationFailed("Get Work Order By ID", "Work order not found");
            return WorkOrderModel();
        }
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get Work Order By ID", e.getMessage());
        return WorkOrderModel();
    }
}

WorkOrderModel WorkOrderService::getWorkOrderByTicketId(const QString& ticketId)
{
    BusinessLogger::businessOperationStart("Get Work Order By Ticket ID", ticketId);
    
    try {
        WorkOrderModel workOrder;
        if (workOrderRepo_->findByTicketId(ticketId, workOrder)) {
            BusinessLogger::businessOperationSuccess("Get Work Order By Ticket ID", ticketId);
            return workOrder;
        } else {
            BusinessLogger::businessOperationFailed("Get Work Order By Ticket ID", "Work order not found");
            return WorkOrderModel();
        }
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get Work Order By Ticket ID", e.getMessage());
        return WorkOrderModel();
    }
}

QList<WorkOrderModel> WorkOrderService::getWorkOrdersByStatus(const QString& status, int limit, int offset)
{
    BusinessLogger::businessOperationStart("Get Work Orders By Status", status);
    
    try {
        QList<WorkOrderModel> workOrders = workOrderRepo_->findByStatus(status);
        BusinessLogger::businessOperationSuccess("Get Work Orders By Status", QString("Found %1 work orders").arg(workOrders.size()));
        return workOrders;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get Work Orders By Status", e.getMessage());
        return QList<WorkOrderModel>();
    }
}

QList<WorkOrderModel> WorkOrderService::getWorkOrdersByCreator(int creatorId, int limit, int offset)
{
    BusinessLogger::businessOperationStart("Get Work Orders By Creator", QString::number(creatorId));
    
    try {
        QList<WorkOrderModel> workOrders = workOrderRepo_->findByCreator(creatorId);
        BusinessLogger::businessOperationSuccess("Get Work Orders By Creator", QString("Found %1 work orders").arg(workOrders.size()));
        return workOrders;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get Work Orders By Creator", e.getMessage());
        return QList<WorkOrderModel>();
    }
}

QList<WorkOrderModel> WorkOrderService::getWorkOrdersByAssignee(int assigneeId, int limit, int offset)
{
    BusinessLogger::businessOperationStart("Get Work Orders By Assignee", QString::number(assigneeId));
    
    try {
        QList<WorkOrderModel> workOrders = workOrderRepo_->findByAssignee(assigneeId);
        BusinessLogger::businessOperationSuccess("Get Work Orders By Assignee", QString("Found %1 work orders").arg(workOrders.size()));
        return workOrders;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get Work Orders By Assignee", e.getMessage());
        return QList<WorkOrderModel>();
    }
}

QList<WorkOrderModel> WorkOrderService::getAllWorkOrders(int limit, int offset)
{
    BusinessLogger::businessOperationStart("Get All Work Orders");
    
    try {
        QList<WorkOrderModel> workOrders = workOrderRepo_->findAll(limit, offset);
        BusinessLogger::businessOperationSuccess("Get All Work Orders", QString("Found %1 work orders").arg(workOrders.size()));
        return workOrders;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get All Work Orders", e.getMessage());
        return QList<WorkOrderModel>();
    }
}

// 工单状态管理
bool WorkOrderService::updateWorkOrderStatus(int workOrderId, const QString& newStatus, int userId)
{
    BusinessLogger::businessOperationStart("Work Order Status Update", QString("Work order: %1, New status: %2").arg(workOrderId).arg(newStatus));
    
    try {
        // 获取当前工单信息
        WorkOrderModel workOrder = getWorkOrderById(workOrderId);
        if (!workOrder.isValid()) {
            BusinessLogger::businessOperationFailed("Work Order Status Update", "Work order not found");
            return false;
        }
        
        // 验证状态转换
        WorkOrderValidator::validateStatusTransition(workOrder.status, newStatus);
        
        // 检查用户权限
        if (!validateUserPermissions(workOrderId, userId, "update_status")) {
            BusinessLogger::businessOperationFailed("Work Order Status Update", "User does not have permission to update status");
            return false;
        }
        
        // 更新状态
        bool success = workOrderRepo_->updateStatus(workOrderId, newStatus);
        
        if (success) {
            BusinessLogger::workOrderStatusChanged(workOrder.ticketId, workOrder.status, newStatus, true);
            BusinessLogger::businessOperationSuccess("Work Order Status Update", QString("Status changed from %1 to %2").arg(workOrder.status).arg(newStatus));
        } else {
            BusinessLogger::workOrderStatusChanged(workOrder.ticketId, workOrder.status, newStatus, false);
            BusinessLogger::businessOperationFailed("Work Order Status Update", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::workOrderStatusChanged("", "", newStatus, false);
        BusinessLogger::businessOperationFailed("Work Order Status Update", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::workOrderStatusChanged("", "", newStatus, false);
        BusinessLogger::businessOperationFailed("Work Order Status Update", e.getMessage());
        return false;
    }
}

bool WorkOrderService::closeWorkOrder(int workOrderId, int userId)
{
    BusinessLogger::businessOperationStart("Work Order Close", QString::number(workOrderId));
    
    try {
        // 获取当前工单信息
        WorkOrderModel workOrder = getWorkOrderById(workOrderId);
        if (!workOrder.isValid()) {
            BusinessLogger::businessOperationFailed("Work Order Close", "Work order not found");
            return false;
        }
        
        // 验证关闭操作
        WorkOrderValidator::validateWorkOrderClose(workOrderId, userId, workOrder.status);
        
        // 检查用户权限（只有创建者可以关闭工单）
        if (workOrder.creatorId != userId) {
            BusinessLogger::businessOperationFailed("Work Order Close", "Only creator can close work order");
            return false;
        }
        
        // 关闭工单
        bool success = workOrderRepo_->updateStatus(workOrderId, WorkOrderStatusManager::CLOSED) &&
                      workOrderRepo_->updateClosedAt(workOrderId, QDateTime::currentDateTime());
        
        if (success) {
            BusinessLogger::workOrderClosed(workOrder.ticketId, userId, true);
            BusinessLogger::businessOperationSuccess("Work Order Close", workOrder.ticketId);
        } else {
            BusinessLogger::workOrderClosed(workOrder.ticketId, userId, false, "Database operation failed");
            BusinessLogger::businessOperationFailed("Work Order Close", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::workOrderClosed("", userId, false, e.getMessage());
        BusinessLogger::businessOperationFailed("Work Order Close", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::workOrderClosed("", userId, false, e.getMessage());
        BusinessLogger::businessOperationFailed("Work Order Close", e.getMessage());
        return false;
    }
}

bool WorkOrderService::startProcessing(int workOrderId, int userId)
{
    return updateWorkOrderStatus(workOrderId, WorkOrderStatusManager::PROCESSING, userId);
}

bool WorkOrderService::refuseWorkOrder(int workOrderId, int userId, const QString& reason)
{
    return updateWorkOrderStatus(workOrderId, WorkOrderStatusManager::REFUSED, userId);
}

// 工单分配
bool WorkOrderService::assignWorkOrder(int workOrderId, int assigneeId, int assignerId)
{
    BusinessLogger::businessOperationStart("Work Order Assignment", QString("Work order: %1, Assignee: %2").arg(workOrderId).arg(assigneeId));
    
    try {
        // 验证分配操作
        WorkOrderValidator::validateAssignment(workOrderId, assigneeId, assignerId);
        
        // 检查工单是否存在
        if (!validateWorkOrderExists(workOrderId)) {
            BusinessLogger::businessOperationFailed("Work Order Assignment", "Work order not found");
            return false;
        }
        
        // 分配工单
        bool success = workOrderRepo_->updateAssignee(workOrderId, assigneeId);
        
        if (success) {
            BusinessLogger::workOrderAssigned("", assigneeId, true);
            BusinessLogger::businessOperationSuccess("Work Order Assignment", QString("Work order %1 assigned to user %2").arg(workOrderId).arg(assigneeId));
        } else {
            BusinessLogger::workOrderAssigned("", assigneeId, false, "Database operation failed");
            BusinessLogger::businessOperationFailed("Work Order Assignment", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::workOrderAssigned("", assigneeId, false, e.getMessage());
        BusinessLogger::businessOperationFailed("Work Order Assignment", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::workOrderAssigned("", assigneeId, false, e.getMessage());
        BusinessLogger::businessOperationFailed("Work Order Assignment", e.getMessage());
        return false;
    }
}

bool WorkOrderService::unassignWorkOrder(int workOrderId, int assignerId)
{
    BusinessLogger::businessOperationStart("Work Order Unassignment", QString::number(workOrderId));
    
    try {
        // 检查工单是否存在
        if (!validateWorkOrderExists(workOrderId)) {
            BusinessLogger::businessOperationFailed("Work Order Unassignment", "Work order not found");
            return false;
        }
        
        // 取消分配工单
        bool success = workOrderRepo_->updateAssignee(workOrderId, -1); // 设置为-1表示未分配
        
        if (success) {
            BusinessLogger::businessOperationSuccess("Work Order Unassignment", QString::number(workOrderId));
        } else {
            BusinessLogger::businessOperationFailed("Work Order Unassignment", "Database operation failed");
        }
        
        return success;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Work Order Unassignment", e.getMessage());
        return false;
    }
}

// 工单参与者管理
bool WorkOrderService::addParticipant(int workOrderId, int userId, const QString& role, const QString& permissions)
{
    BusinessLogger::businessOperationStart("Add Work Order Participant", QString("Work order: %1, User: %2, Role: %3").arg(workOrderId).arg(userId).arg(role));
    
    try {
        // 验证参与者添加
        WorkOrderValidator::validateParticipantAddition(workOrderId, userId, role);
        
        // 检查工单是否存在
        if (!validateWorkOrderExists(workOrderId)) {
            BusinessLogger::businessOperationFailed("Add Work Order Participant", "Work order not found");
            return false;
        }
        
        // 添加参与者
        bool success = workOrderRepo_->addParticipant(workOrderId, userId, role, permissions);
        
        if (success) {
            BusinessLogger::businessOperationSuccess("Add Work Order Participant", QString("User %1 added to work order %2").arg(userId).arg(workOrderId));
        } else {
            BusinessLogger::businessOperationFailed("Add Work Order Participant", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::businessOperationFailed("Add Work Order Participant", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Add Work Order Participant", e.getMessage());
        return false;
    }
}

bool WorkOrderService::removeParticipant(int workOrderId, int userId)
{
    BusinessLogger::businessOperationStart("Remove Work Order Participant", QString("Work order: %1, User: %2").arg(workOrderId).arg(userId));
    
    try {
        // 检查工单是否存在
        if (!validateWorkOrderExists(workOrderId)) {
            BusinessLogger::businessOperationFailed("Remove Work Order Participant", "Work order not found");
            return false;
        }
        
        // 移除参与者
        bool success = workOrderRepo_->removeParticipant(workOrderId, userId);
        
        if (success) {
            BusinessLogger::businessOperationSuccess("Remove Work Order Participant", QString("User %1 removed from work order %2").arg(userId).arg(workOrderId));
        } else {
            BusinessLogger::businessOperationFailed("Remove Work Order Participant", "Database operation failed");
        }
        
        return success;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Remove Work Order Participant", e.getMessage());
        return false;
    }
}

bool WorkOrderService::updateParticipantRole(int workOrderId, int userId, const QString& newRole)
{
    BusinessLogger::businessOperationStart("Update Participant Role", QString("Work order: %1, User: %2, New role: %3").arg(workOrderId).arg(userId).arg(newRole));
    
    try {
        // 验证角色
        WorkOrderValidator::validateRole(newRole);
        
        // 检查工单是否存在
        if (!validateWorkOrderExists(workOrderId)) {
            BusinessLogger::businessOperationFailed("Update Participant Role", "Work order not found");
            return false;
        }
        
        // 更新参与者角色
        bool success = workOrderRepo_->updateParticipantRole(workOrderId, userId, newRole);
        
        if (success) {
            BusinessLogger::businessOperationSuccess("Update Participant Role", QString("User %1 role updated to %2").arg(userId).arg(newRole));
        } else {
            BusinessLogger::businessOperationFailed("Update Participant Role", "Database operation failed");
        }
        
        return success;
    }
    catch (const ValidationException& e) {
        BusinessLogger::businessOperationFailed("Update Participant Role", e.getMessage());
        return false;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Update Participant Role", e.getMessage());
        return false;
    }
}

QList<ParticipantModel> WorkOrderService::getWorkOrderParticipants(int workOrderId)
{
    BusinessLogger::businessOperationStart("Get Work Order Participants", QString::number(workOrderId));
    
    try {
        QList<ParticipantModel> participants = workOrderRepo_->getParticipants(workOrderId);
        BusinessLogger::businessOperationSuccess("Get Work Order Participants", QString("Found %1 participants").arg(participants.size()));
        return participants;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get Work Order Participants", e.getMessage());
        return QList<ParticipantModel>();
    }
}

bool WorkOrderService::isParticipant(int workOrderId, int userId)
{
    return workOrderRepo_->isParticipant(workOrderId, userId);
}

// 工单统计
int WorkOrderService::getWorkOrderCount(const QString& status)
{
    BusinessLogger::businessOperationStart("Get Work Order Count", status.isEmpty() ? "All" : status);
    
    try {
        int count = status.isEmpty() ? workOrderRepo_->countAll() : workOrderRepo_->countByStatus(status);
        BusinessLogger::businessOperationSuccess("Get Work Order Count", QString::number(count));
        return count;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Get Work Order Count", e.getMessage());
        return 0;
    }
}

int WorkOrderService::getWorkOrderCountByCreator(int creatorId)
{
    return workOrderRepo_->countByCreator(creatorId);
}

int WorkOrderService::getWorkOrderCountByAssignee(int assigneeId)
{
    return workOrderRepo_->countByAssignee(assigneeId);
}

// 权限检查
bool WorkOrderService::canModifyWorkOrder(int workOrderId, int userId)
{
    // 这里可以实现更复杂的权限检查逻辑
    // 目前简单返回true，后续可以扩展
    return true;
}

bool WorkOrderService::canCloseWorkOrder(int workOrderId, int userId)
{
    WorkOrderModel workOrder = getWorkOrderById(workOrderId);
    if (!workOrder.isValid()) {
        return false;
    }
    
    // 只有创建者可以关闭工单
    return workOrder.creatorId == userId;
}

bool WorkOrderService::canAssignWorkOrder(int workOrderId, int userId)
{
    // 这里可以实现更复杂的权限检查逻辑
    // 目前简单返回true，后续可以扩展
    return true;
}

// 状态查询
QStringList WorkOrderService::getNextPossibleStatuses(int workOrderId)
{
    WorkOrderModel workOrder = getWorkOrderById(workOrderId);
    if (!workOrder.isValid()) {
        return QStringList();
    }
    
    return WorkOrderStatusManager::getNextPossibleStatuses(workOrder.status);
}

bool WorkOrderService::canTransitionTo(int workOrderId, const QString& targetStatus)
{
    WorkOrderModel workOrder = getWorkOrderById(workOrderId);
    if (!workOrder.isValid()) {
        return false;
    }
    
    return WorkOrderStatusManager::isValidTransition(workOrder.status, targetStatus);
}

// 私有辅助方法
bool WorkOrderService::validateWorkOrderExists(int workOrderId)
{
    WorkOrderModel workOrder;
    return workOrderRepo_->findById(workOrderId, workOrder);
}

bool WorkOrderService::validateUserPermissions(int workOrderId, int userId, const QString& operation)
{
    // 这里可以实现更复杂的权限检查逻辑
    // 目前简单返回true，后续可以扩展
    return true;
}

void WorkOrderService::logWorkOrderActivity(const QString& operation, int workOrderId, bool success, const QString& details)
{
    if (success) {
        BusinessLogger::info(operation, QString("Work order %1 operation completed successfully").arg(workOrderId));
    } else {
        QString message = QString("Work order %1 operation failed").arg(workOrderId);
        if (!details.isEmpty()) {
            message += QString(": %1").arg(details);
        }
        BusinessLogger::warning(operation, message);
    }
}

QString WorkOrderService::generateTicketId()
{
    // 生成工单号的逻辑
    // 这里可以基于时间戳或其他规则生成唯一工单号
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMddhhmmss");
    QString randomSuffix = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    return QString("WO%1%2").arg(timestamp).arg(randomSuffix);
}

// 业务验证方法
bool WorkOrderService::validateWorkOrderCreation(const QString& title, const QString& description, int creatorId)
{
    // 检查创建者是否存在
    if (creatorId <= 0) {
        BusinessLogger::validationFailed("Work Order Creation", "creatorId", "Invalid creator ID");
        return false;
    }
    
    // 检查用户是否有创建工单的权限
    if (!checkUserPermission(creatorId, "create_workorder", 0)) {
        BusinessLogger::validationFailed("Work Order Creation", "permission", "User does not have permission to create work orders");
        return false;
    }
    
    return true;
}

bool WorkOrderService::validateStatusTransition(int workOrderId, const QString& newStatus, int userId)
{
    // 检查工单是否存在
    WorkOrderModel workOrder = getWorkOrderById(workOrderId);
    if (!workOrder.isValid()) {
        BusinessLogger::validationFailed("Status Transition", "workOrderId", "Work order not found");
        return false;
    }
    
    // 检查状态转换是否有效
    if (!WorkOrderStatusManager::isValidTransition(workOrder.status, newStatus)) {
        BusinessLogger::validationFailed("Status Transition", "status", 
            QString("Invalid transition from '%1' to '%2'").arg(workOrder.status).arg(newStatus));
        return false;
    }
    
    // 检查用户权限
    if (!checkUserPermission(userId, "update_status", workOrderId)) {
        BusinessLogger::validationFailed("Status Transition", "permission", "User does not have permission to update status");
        return false;
    }
    
    return true;
}

bool WorkOrderService::validateAssignment(int workOrderId, int assigneeId, int assignerId)
{
    // 检查工单是否存在
    WorkOrderModel workOrder = getWorkOrderById(workOrderId);
    if (!workOrder.isValid()) {
        BusinessLogger::validationFailed("Assignment", "workOrderId", "Work order not found");
        return false;
    }
    
    // 检查分配者权限
    if (!checkUserPermission(assignerId, "assign_workorder", workOrderId)) {
        BusinessLogger::validationFailed("Assignment", "permission", "Assigner does not have permission to assign work orders");
        return false;
    }
    
    // 检查被分配者是否存在且是专家类型
    // 这里可以添加更多的业务规则验证
    
    return true;
}

bool WorkOrderService::validateParticipantAddition(int workOrderId, int userId, const QString& role)
{
    // 检查工单是否存在
    WorkOrderModel workOrder = getWorkOrderById(workOrderId);
    if (!workOrder.isValid()) {
        BusinessLogger::validationFailed("Participant Addition", "workOrderId", "Work order not found");
        return false;
    }
    
    // 检查用户是否已经是参与者
    if (isParticipant(workOrderId, userId)) {
        BusinessLogger::validationFailed("Participant Addition", "userId", "User is already a participant");
        return false;
    }
    
    // 检查角色是否有效
    if (role != ParticipantModel::ROLE_CREATOR && 
        role != ParticipantModel::ROLE_EXPERT && 
        role != ParticipantModel::ROLE_OPERATOR && 
        role != ParticipantModel::ROLE_VIEWER) {
        BusinessLogger::validationFailed("Participant Addition", "role", "Invalid role");
        return false;
    }
    
    return true;
}

// 权限检查方法
bool WorkOrderService::checkUserPermission(int userId, const QString& operation, int workOrderId)
{
    // 这里应该调用权限管理器
    // 目前简单返回true，后续可以扩展
    return true;
}

bool WorkOrderService::checkWorkOrderAccess(int userId, int workOrderId)
{
    // 检查用户是否有访问工单的权限
    WorkOrderModel workOrder = getWorkOrderById(workOrderId);
    if (!workOrder.isValid()) {
        return false;
    }
    
    // 创建者、分配者、参与者都可以访问
    if (workOrder.creatorId == userId || workOrder.assignedTo == userId) {
        return true;
    }
    
    return isParticipant(workOrderId, userId);
}

// 业务事件触发方法
void WorkOrderService::triggerWorkOrderCreatedEvent(const WorkOrderModel& workOrder)
{
    // 这里应该调用事件分发器
    // 目前只是记录日志，后续可以扩展
    BusinessLogger::eventTriggered("workorder.created", "WorkOrderService", 
        QJsonObject{{"workOrderId", workOrder.id}, {"ticketId", workOrder.ticketId}});
}

void WorkOrderService::triggerWorkOrderStatusChangedEvent(const WorkOrderModel& workOrder, const QString& oldStatus)
{
    // 这里应该调用事件分发器
    BusinessLogger::eventTriggered("workorder.status.changed", "WorkOrderService", 
        QJsonObject{
            {"workOrderId", workOrder.id}, 
            {"ticketId", workOrder.ticketId},
            {"oldStatus", oldStatus},
            {"newStatus", workOrder.status}
        });
}

void WorkOrderService::triggerWorkOrderAssignedEvent(const WorkOrderModel& workOrder, int assigneeId)
{
    // 这里应该调用事件分发器
    BusinessLogger::eventTriggered("workorder.assigned", "WorkOrderService", 
        QJsonObject{
            {"workOrderId", workOrder.id}, 
            {"ticketId", workOrder.ticketId},
            {"assigneeId", assigneeId}
        });
}

void WorkOrderService::triggerWorkOrderClosedEvent(const WorkOrderModel& workOrder)
{
    // 这里应该调用事件分发器
    BusinessLogger::eventTriggered("workorder.closed", "WorkOrderService", 
        QJsonObject{
            {"workOrderId", workOrder.id}, 
            {"ticketId", workOrder.ticketId}
        });
}
