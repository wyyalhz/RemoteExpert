#include "workorder_handler.h"
#include "../connection_manager.h"
#include "../logging/network_logger.h"
#include "../../../common/protocol/protocol.h"

WorkOrderHandler::WorkOrderHandler(WorkOrderService* workOrderService, QObject *parent)
    : ProtocolHandler(parent)
    , workOrderService_(workOrderService)
{
}

WorkOrderHandler::~WorkOrderHandler()
{
}

void WorkOrderHandler::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    // 检查认证状态
    if (!checkAuthentication(socket)) {
        return;
    }
    
    switch (packet.type) {
        case MSG_CREATE_WORKORDER:
            handleCreateWorkOrder(socket, packet.json);
            break;
        case MSG_JOIN_WORKORDER:
            handleJoinWorkOrder(socket, packet.json);
            break;
        case MSG_LEAVE_WORKORDER:
            handleLeaveWorkOrder(socket, packet.json);
            break;
        case MSG_UPDATE_WORKORDER:
            handleUpdateWorkOrder(socket, packet.json);
            break;
        case MSG_LIST_WORKORDERS:
            handleListWorkOrders(socket, packet.json);
            break;
        default:
            sendErrorResponse(socket, MSG_ERROR, 404, QString("Unknown work order message type: %1").arg(packet.type));
            break;
    }
}

void WorkOrderHandler::handleCreateWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    // 使用MessageValidator验证创建工单消息
    QString validationError;
    if (!MessageValidator::validateCreateWorkOrderMessage(data, validationError)) {
        sendErrorResponse(socket, MSG_CREATE_WORKORDER, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析创建工单消息
    QString title, description, category;
    int priority;
    QJsonObject deviceInfo;
    if (!MessageParser::parseCreateWorkOrderMessage(data, title, description, priority, category, deviceInfo)) {
        sendErrorResponse(socket, MSG_CREATE_WORKORDER, 400, "Invalid create work order message format");
        return;
    }
    
    int creatorId = getUserIdFromContext(socket);
    if (creatorId <= 0) {
        sendErrorResponse(socket, MSG_CREATE_WORKORDER, 400, "Invalid user context");
        return;
    }
    
    // 将数字优先级转换为字符串优先级
    QString priorityString = convertPriorityToString(priority);
    
    // 调用业务服务创建工单
    QString generatedTicketId;
    bool success = workOrderService_->createWorkOrder(title, description, creatorId, priorityString, category, generatedTicketId);
    
    if (success) {
        QJsonObject responseData = MessageBuilder::buildWorkOrderCreatedResponse(
            generatedTicketId, title, priorityString, category);
        sendSuccessResponse(socket, MSG_CREATE_WORKORDER, "Work order created successfully", responseData);
        
        NetworkLogger::info("Work Order Handler", 
                           QString("Work order '%1' created successfully by user %2")
                           .arg(generatedTicketId).arg(creatorId));
    } else {
        sendErrorResponse(socket, MSG_CREATE_WORKORDER, 500, "Failed to create work order");
        
        NetworkLogger::error("Work Order Handler", 
                            QString("Failed to create work order for user %1")
                            .arg(creatorId));
    }
}

void WorkOrderHandler::handleJoinWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    // 使用MessageValidator验证加入工单消息
    QString validationError;
    if (!MessageValidator::validateJoinWorkOrderMessage(data, validationError)) {
        sendErrorResponse(socket, MSG_JOIN_WORKORDER, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析加入工单消息
    QString roomId, role;
    if (!MessageParser::parseJoinWorkOrderMessage(data, roomId, role)) {
        sendErrorResponse(socket, MSG_JOIN_WORKORDER, 400, "Invalid join work order message format");
        return;
    }
    
    // 检查工单是否存在
    WorkOrderModel workOrder = workOrderService_->getWorkOrderByTicketId(roomId);
    if (!workOrder.isValid()) {
        sendErrorResponse(socket, MSG_JOIN_WORKORDER, 404, "Work order not found");
        return;
    }
    
    // 获取用户ID
    int userId = getUserIdFromContext(socket);
    if (userId <= 0) {
        sendErrorResponse(socket, MSG_JOIN_WORKORDER, 400, "Invalid user context");
        return;
    }
    
    // 加入房间并更新会话
    if (getConnectionManager()) {
        getConnectionManager()->joinRoom(socket, roomId);
        
        // 更新用户会话到工单房间
        getConnectionManager()->createSessionForUser(socket, userId, roomId);
        
        QJsonObject responseData = MessageBuilder::buildWorkOrderJoinedResponse(
            roomId, workOrder.toJson());
        sendSuccessResponse(socket, MSG_JOIN_WORKORDER, "Joined work order successfully", responseData);
        
        NetworkLogger::info("Work Order Handler", 
                           QString("User %1 joined work order '%2'")
                           .arg(userId).arg(roomId));
    } else {
        sendErrorResponse(socket, MSG_JOIN_WORKORDER, 500, "Connection manager not available");
    }
}

void WorkOrderHandler::handleGetWorkOrderInfo(QTcpSocket* socket, const QJsonObject& data)
{
    QString ticketId = data.value("ticket_id").toString();
    int workOrderId = data.value("work_order_id").toInt();
    
    WorkOrderModel workOrder;
    
    if (!ticketId.isEmpty()) {
        workOrder = workOrderService_->getWorkOrderByTicketId(ticketId);
    } else if (workOrderId > 0) {
        workOrder = workOrderService_->getWorkOrderById(workOrderId);
    } else {
        sendErrorResponse(socket, MSG_ERROR, 400, "Either ticket_id or work_order_id must be provided");
        return;
    }
    
    if (!workOrder.isValid()) {
        sendErrorResponse(socket, MSG_ERROR, 404, "Work order not found");
        return;
    }
    
            sendSuccessResponse(socket, MSG_ERROR, "Work order info retrieved", workOrder.toJson());
}

void WorkOrderHandler::handleUpdateWorkOrderStatus(QTcpSocket* socket, const QJsonObject& data)
{
    int workOrderId = data.value("work_order_id").toInt();
    QString newStatus = data.value("status").toString();
    
    if (workOrderId <= 0 || newStatus.isEmpty()) {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 400, "Work order ID and status are required");
        return;
    }
    
    int userId = getUserIdFromContext(socket);
    bool success = workOrderService_->updateWorkOrderStatus(workOrderId, newStatus, userId);
    
    if (success) {
        sendSuccessResponse(socket, MSG_UPDATE_WORKORDER, "Work order status updated successfully");
    } else {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 500, "Failed to update work order status");
    }
}

void WorkOrderHandler::handleAssignWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    int workOrderId = data.value("work_order_id").toInt();
    int assigneeId = data.value("assignee_id").toInt();
    
    if (workOrderId <= 0 || assigneeId <= 0) {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 400, "Work order ID and assignee ID are required");
        return;
    }
    
    int assignerId = getUserIdFromContext(socket);
    bool success = workOrderService_->assignWorkOrder(workOrderId, assigneeId, assignerId);
    
    if (success) {
        sendSuccessResponse(socket, MSG_UPDATE_WORKORDER, "Work order assigned successfully");
    } else {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 500, "Failed to assign work order");
    }
}

void WorkOrderHandler::handleCloseWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    int workOrderId = data.value("work_order_id").toInt();
    
    if (workOrderId <= 0) {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 400, "Work order ID is required");
        return;
    }
    
    int userId = getUserIdFromContext(socket);
    bool success = workOrderService_->closeWorkOrder(workOrderId, userId);
    
    if (success) {
        sendSuccessResponse(socket, MSG_UPDATE_WORKORDER, "Work order closed successfully");
    } else {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 500, "Failed to close work order");
    }
}

void WorkOrderHandler::handleGetWorkOrderList(QTcpSocket* socket, const QJsonObject& data)
{
    QString status = data.value("status").toString();
    int limit = data.value("limit").toInt(50);
    int offset = data.value("offset").toInt(0);
    
    QList<WorkOrderModel> workOrders;
    
    if (!status.isEmpty()) {
        workOrders = workOrderService_->getWorkOrdersByStatus(status, limit, offset);
    } else {
        workOrders = workOrderService_->getAllWorkOrders(limit, offset);
    }
    
    QJsonArray workOrderArray;
    for (const WorkOrderModel& workOrder : workOrders) {
        workOrderArray.append(workOrder.toJson());
    }
    
    QJsonObject responseData = MessageBuilder::buildWorkOrderListResponse(
        workOrderArray, workOrderArray.size());
    
    sendSuccessResponse(socket, MSG_LIST_WORKORDERS, "Work order list retrieved", responseData);
}

int WorkOrderHandler::getUserIdFromContext(QTcpSocket* socket)
{
    ClientContext* context = getClientContext(socket);
    if (context && context->isAuthenticated && context->userId > 0) {
        return context->userId;
    }
    
    return -1;
}

QString WorkOrderHandler::convertPriorityToString(int priority)
{
    // 将数字优先级转换为字符串优先级
    switch (priority) {
        case 1:
            return "low";
        case 2:
            return "normal";
        case 3:
            return "high";
        default:
            // 默认返回中等优先级
            return "normal";
    }
}

void WorkOrderHandler::handleLeaveWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    // 使用MessageValidator验证离开工单消息
    QString validationError;
    if (!MessageValidator::validateLeaveWorkOrderMessage(data, validationError)) {
        sendErrorResponse(socket, MSG_LEAVE_WORKORDER, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析离开工单消息
    QString roomId;
    if (!MessageParser::parseLeaveWorkOrderMessage(data, roomId)) {
        sendErrorResponse(socket, MSG_LEAVE_WORKORDER, 400, "Invalid leave work order message format");
        return;
    }
    
    // 离开房间
    if (getConnectionManager()) {
        getConnectionManager()->leaveRoom(socket);
        
        QJsonObject responseData = MessageBuilder::buildWorkOrderLeftResponse(roomId);
        sendSuccessResponse(socket, MSG_LEAVE_WORKORDER, "Left work order successfully", responseData);
        
        NetworkLogger::info("Work Order Handler", 
                           QString("User left work order '%1'")
                           .arg(roomId));
    } else {
        sendErrorResponse(socket, MSG_LEAVE_WORKORDER, 500, "Connection manager not available");
    }
}

void WorkOrderHandler::handleUpdateWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    // 使用MessageValidator验证更新工单消息
    QString validationError;
    if (!MessageValidator::validateUpdateWorkOrderMessage(data, validationError)) {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析更新工单消息
    QString ticketId, status, description;
    if (!MessageParser::parseUpdateWorkOrderMessage(data, ticketId, status, description)) {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 400, "Invalid update work order message format");
        return;
    }
    
    int userId = getUserIdFromContext(socket);
    if (userId <= 0) {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 400, "Invalid user context");
        return;
    }
    
    // 获取现有工单并更新
    WorkOrderModel workOrder = workOrderService_->getWorkOrderByTicketId(ticketId);
    if (!workOrder.isValid()) {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 404, "Work order not found");
        return;
    }
    
    // 更新工单字段
    if (!status.isEmpty()) workOrder.status = status;
    if (!description.isEmpty()) workOrder.description = description;
    
    // 调用业务服务更新工单
    bool success = workOrderService_->updateWorkOrder(workOrder);
    
    if (success) {
        QJsonObject responseData = MessageBuilder::buildWorkOrderUpdatedResponse(ticketId, status);
        sendSuccessResponse(socket, MSG_UPDATE_WORKORDER, "Work order updated successfully", responseData);
        
        NetworkLogger::info("Work Order Handler", 
                           QString("Work order '%1' updated by user %2")
                           .arg(ticketId).arg(userId));
    } else {
        sendErrorResponse(socket, MSG_UPDATE_WORKORDER, 500, "Failed to update work order");
        
        NetworkLogger::error("Work Order Handler", 
                            QString("Failed to update work order '%1' by user %2")
                            .arg(ticketId).arg(userId));
    }
}

void WorkOrderHandler::handleListWorkOrders(QTcpSocket* socket, const QJsonObject& data)
{
    // 使用MessageValidator验证获取工单列表消息
    QString validationError;
    if (!MessageValidator::validateListWorkOrdersMessage(data, validationError)) {
        sendErrorResponse(socket, MSG_LIST_WORKORDERS, 400, validationError);
        return;
    }
    
    // 使用MessageParser解析获取工单列表消息
    QString status;
    int limit, offset;
    if (!MessageParser::parseListWorkOrdersMessage(data, status, limit, offset)) {
        sendErrorResponse(socket, MSG_LIST_WORKORDERS, 400, "Invalid list work orders message format");
        return;
    }
    
    int userId = getUserIdFromContext(socket);
    if (userId <= 0) {
        sendErrorResponse(socket, MSG_LIST_WORKORDERS, 400, "Invalid user context");
        return;
    }
    
    // 调用业务服务获取工单列表
    QList<WorkOrderModel> workOrders = workOrderService_->getWorkOrdersByCreator(userId);
    
    QJsonArray workOrderArray;
    for (const auto& workOrder : workOrders) {
        workOrderArray.append(workOrder.toJson());
    }
    
    QJsonObject responseData = MessageBuilder::buildWorkOrderListResponse(
        workOrderArray, workOrderArray.size());
    sendSuccessResponse(socket, MSG_LIST_WORKORDERS, "Work orders retrieved successfully", responseData);
    
    NetworkLogger::info("Work Order Handler", 
                       QString("Retrieved %1 work orders for user %2")
                       .arg(workOrderArray.size()).arg(userId));
}
