#include "workorder_validator.h"
#include "../logging/business_logger.h"

void WorkOrderValidator::validateCreation(const QString& title, const QString& description, 
                                       int creatorId, const QString& priority, const QString& category)
{
    BusinessLogger::businessOperationStart("Work Order Creation Validation");
    
    try {
        validateTitle(title);
        validateDescription(description);
        validatePriority(priority);
        validateCategory(category);
        
        if (creatorId <= 0) {
            throw ValidationException("Creator ID must be positive", "creatorId", "Work Order Creation");
        }
        
        BusinessLogger::validationSuccess("Work Order Creation", "All fields validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("Work Order Creation", e.getField(), e.getMessage());
        throw;
    }
}

void WorkOrderValidator::validateUpdate(const WorkOrderModel& workOrder)
{
    BusinessLogger::businessOperationStart("Work Order Update Validation");
    
    try {
        if (workOrder.id <= 0) {
            throw ValidationException("Work order ID must be positive", "id", "Work Order Update");
        }
        
        validateWorkOrderModel(workOrder);
        
        BusinessLogger::validationSuccess("Work Order Update", "Work order update data validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("Work Order Update", e.getField(), e.getMessage());
        throw;
    }
}

void WorkOrderValidator::validateWorkOrderModel(const WorkOrderModel& workOrder)
{
    BusinessLogger::businessOperationStart("Work Order Model Validation");
    
    try {
        if (!workOrder.isValid()) {
            throw ValidationException("Work order model is invalid", "model", "Work Order Model Validation");
        }
        
        validateTitle(workOrder.title);
        validateDescription(workOrder.description);
        validatePriority(workOrder.priority);
        validateStatus(workOrder.status);
        
        if (!workOrder.category.isEmpty()) {
            validateCategory(workOrder.category);
        }
        
        BusinessLogger::validationSuccess("Work Order Model", "Work order model validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("Work Order Model", e.getField(), e.getMessage());
        throw;
    }
}

void WorkOrderValidator::validateStatusTransition(const QString& currentStatus, const QString& newStatus)
{
    BusinessLogger::businessOperationStart("Work Order Status Transition Validation");
    
    try {
        validateStatus(currentStatus);
        validateStatus(newStatus);
        
        if (!WorkOrderStatusManager::isValidTransition(currentStatus, newStatus)) {
            throw StateTransitionException(currentStatus, newStatus, "Work Order Status Transition");
        }
        
        BusinessLogger::validationSuccess("Work Order Status Transition", 
                                        QString("Status transition from '%1' to '%2' is valid").arg(currentStatus).arg(newStatus));
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("Work Order Status Transition", e.getField(), e.getMessage());
        throw;
    }
}

void WorkOrderValidator::validateAssignment(int workOrderId, int assigneeId, int assignerId)
{
    BusinessLogger::businessOperationStart("Work Order Assignment Validation");
    
    try {
        if (workOrderId <= 0) {
            throw ValidationException("Work order ID must be positive", "workOrderId", "Work Order Assignment");
        }
        
        if (assigneeId <= 0) {
            throw ValidationException("Assignee ID must be positive", "assigneeId", "Work Order Assignment");
        }
        
        if (assignerId <= 0) {
            throw ValidationException("Assigner ID must be positive", "assignerId", "Work Order Assignment");
        }
        
        if (assigneeId == assignerId) {
            throw ValidationException("Assignee cannot be the same as assigner", "assigneeId", "Work Order Assignment");
        }
        
        BusinessLogger::validationSuccess("Work Order Assignment", "Assignment data validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("Work Order Assignment", e.getField(), e.getMessage());
        throw;
    }
}

void WorkOrderValidator::validateParticipantAddition(int workOrderId, int userId, const QString& role)
{
    BusinessLogger::businessOperationStart("Work Order Participant Addition Validation");
    
    try {
        if (workOrderId <= 0) {
            throw ValidationException("Work order ID must be positive", "workOrderId", "Participant Addition");
        }
        
        if (userId <= 0) {
            throw ValidationException("User ID must be positive", "userId", "Participant Addition");
        }
        
        validateRole(role);
        
        BusinessLogger::validationSuccess("Work Order Participant Addition", "Participant addition data validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("Work Order Participant Addition", e.getField(), e.getMessage());
        throw;
    }
}

void WorkOrderValidator::validateWorkOrderClose(int workOrderId, int userId, const QString& currentStatus)
{
    BusinessLogger::businessOperationStart("Work Order Close Validation");
    
    try {
        if (workOrderId <= 0) {
            throw ValidationException("Work order ID must be positive", "workOrderId", "Work Order Close");
        }
        
        if (userId <= 0) {
            throw ValidationException("User ID must be positive", "userId", "Work Order Close");
        }
        
        validateStatus(currentStatus);
        
        if (!WorkOrderStatusManager::canCloseWorkOrder(currentStatus)) {
            throw ValidationException("Cannot close work order in current status", "status", "Work Order Close");
        }
        
        BusinessLogger::validationSuccess("Work Order Close", "Work order close data validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("Work Order Close", e.getField(), e.getMessage());
        throw;
    }
}

void WorkOrderValidator::validateTitle(const QString& title)
{
    if (title.isEmpty()) {
        throw ValidationException("Title cannot be empty", "title", "Title Validation");
    }
    
    if (title.length() < 5) {
        throw ValidationException("Title must be at least 5 characters long", "title", "Title Validation");
    }
    
    if (title.length() > 200) {
        throw ValidationException("Title cannot exceed 200 characters", "title", "Title Validation");
    }
}

void WorkOrderValidator::validateDescription(const QString& description)
{
    if (description.isEmpty()) {
        throw ValidationException("Description cannot be empty", "description", "Description Validation");
    }
    
    if (description.length() < 10) {
        throw ValidationException("Description must be at least 10 characters long", "description", "Description Validation");
    }
    
    if (description.length() > 2000) {
        throw ValidationException("Description cannot exceed 2000 characters", "description", "Description Validation");
    }
}

void WorkOrderValidator::validatePriority(const QString& priority)
{
    if (priority.isEmpty()) {
        throw ValidationException("Priority cannot be empty", "priority", "Priority Validation");
    }
    
    if (!isValidPriority(priority)) {
        throw ValidationException("Invalid priority value", "priority", "Priority Validation");
    }
}

void WorkOrderValidator::validateCategory(const QString& category)
{
    if (category.isEmpty()) {
        throw ValidationException("Category cannot be empty", "category", "Category Validation");
    }
    
    if (category.length() > 100) {
        throw ValidationException("Category cannot exceed 100 characters", "category", "Category Validation");
    }
}

void WorkOrderValidator::validateStatus(const QString& status)
{
    if (status.isEmpty()) {
        throw ValidationException("Status cannot be empty", "status", "Status Validation");
    }
    
    if (!WorkOrderStatusManager::isValidStatus(status)) {
        throw ValidationException("Invalid status value", "status", "Status Validation");
    }
}

void WorkOrderValidator::validateRole(const QString& role)
{
    if (role.isEmpty()) {
        throw ValidationException("Role cannot be empty", "role", "Role Validation");
    }
    
    if (!isValidRole(role)) {
        throw ValidationException("Invalid role value", "role", "Role Validation");
    }
}

// 私有辅助方法
bool WorkOrderValidator::isValidPriority(const QString& priority)
{
    return priority == WorkOrderModel::PRIORITY_LOW ||
           priority == WorkOrderModel::PRIORITY_NORMAL ||
           priority == WorkOrderModel::PRIORITY_HIGH ||
           priority == WorkOrderModel::PRIORITY_URGENT;
}

bool WorkOrderValidator::isValidRole(const QString& role)
{
    return role == ParticipantModel::ROLE_CREATOR ||
           role == ParticipantModel::ROLE_EXPERT ||
           role == ParticipantModel::ROLE_OPERATOR ||
           role == ParticipantModel::ROLE_VIEWER;
}
