#ifndef WORKORDER_VALIDATOR_H
#define WORKORDER_VALIDATOR_H

#include "../exceptions/business_exception.h"
#include "../managers/workorder_status_manager.h"
#include "../../data/models/workorder_model.h"
#include <QString>
#include <QJsonObject>

// 工单数据验证器
class WorkOrderValidator
{
public:
    // 验证工单创建数据
    static void validateCreation(const QString& title, const QString& description, 
                               int creatorId, const QString& priority, const QString& category);
    
    // 验证工单更新数据
    static void validateUpdate(const WorkOrderModel& workOrder);
    
    // 验证工单模型
    static void validateWorkOrderModel(const WorkOrderModel& workOrder);
    
    // 验证工单状态转换（使用WorkOrderStatusManager）
    static void validateStatusTransition(const QString& currentStatus, const QString& newStatus);
    
    // 验证工单分配
    static void validateAssignment(int workOrderId, int assigneeId, int assignerId);
    
    // 验证参与者添加
    static void validateParticipantAddition(int workOrderId, int userId, const QString& role);
    
    // 验证工单关闭
    static void validateWorkOrderClose(int workOrderId, int userId, const QString& currentStatus);
    
    // 验证标题
    static void validateTitle(const QString& title);
    
    // 验证描述
    static void validateDescription(const QString& description);
    
    // 验证优先级
    static void validatePriority(const QString& priority);
    
    // 验证分类
    static void validateCategory(const QString& category);
    
    // 验证状态（使用WorkOrderStatusManager）
    static void validateStatus(const QString& status);
    
    // 验证角色
    static void validateRole(const QString& role);

private:
    // 私有辅助方法
    static bool isValidPriority(const QString& priority);
    static bool isValidRole(const QString& role);
};

#endif // WORKORDER_VALIDATOR_H
