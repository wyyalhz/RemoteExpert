#include "workorder_status_manager.h"
#include "../logging/business_logger.h"

// 静态常量定义
const QString WorkOrderStatusManager::OPEN = "open";
const QString WorkOrderStatusManager::PROCESSING = "processing";
const QString WorkOrderStatusManager::REFUSED = "refused";
const QString WorkOrderStatusManager::CLOSED = "closed";

// 状态等级映射
int WorkOrderStatusManager::getStatusLevel(const QString &status)
{
    if (status == OPEN) return 0;
    if (status == REFUSED) return 3;
    if (status == PROCESSING) return 6;
    if (status == CLOSED) return 9;
    return -1; // 无效状态
}

// 状态转换合法性验证
bool WorkOrderStatusManager::isValidTransition(const QString &fromStatus, const QString &toStatus)
{
    // 获取状态等级
    int fromLevel = getStatusLevel(fromStatus);
    int toLevel = getStatusLevel(toStatus);
    
    // 检查状态是否有效
    if (fromLevel == -1 || toLevel == -1) {
        BusinessLogger::warning("Status Transition", 
                              QString("Invalid status: from '%1' (level: %2) to '%3' (level: %4)")
                              .arg(fromStatus).arg(fromLevel).arg(toStatus).arg(toLevel));
        return false;
    }
    
    // 只能从低等级状态转换为高等级状态
    bool isValid = toLevel > fromLevel;
    
    if (!isValid) {
        BusinessLogger::warning("Status Transition", 
                              QString("Invalid transition: cannot go from '%1' (level: %2) to '%3' (level: %4)")
                              .arg(fromStatus).arg(fromLevel).arg(toStatus).arg(toLevel));
    }
    
    return isValid;
}

// 获取所有有效状态
QStringList WorkOrderStatusManager::getValidStatuses()
{
    return {OPEN, REFUSED, PROCESSING, CLOSED};
}

// 验证状态是否有效
bool WorkOrderStatusManager::isValidStatus(const QString &status)
{
    return getStatusLevel(status) != -1;
}

// 获取状态描述
QString WorkOrderStatusManager::getStatusDescription(const QString &status)
{
    if (status == OPEN) return "待处理";
    if (status == PROCESSING) return "处理中";
    if (status == REFUSED) return "已拒绝";
    if (status == CLOSED) return "已关闭";
    return "未知状态";
}

// 获取下一个可能的状态列表
QStringList WorkOrderStatusManager::getNextPossibleStatuses(const QString &currentStatus)
{
    QStringList possibleStatuses;
    
    if (currentStatus == OPEN) {
        possibleStatuses = QStringList() << PROCESSING << REFUSED << CLOSED;
    }
    else if (currentStatus == PROCESSING) {
        possibleStatuses = QStringList() << REFUSED << CLOSED;
    }
    else if (currentStatus == REFUSED) {
        possibleStatuses = QStringList() << OPEN << CLOSED;
    }
    else if (currentStatus == CLOSED) {
        // 已关闭的工单不能转换状态
        possibleStatuses = QStringList();
    }
    
    return possibleStatuses;
}

// 检查是否可以关闭工单
bool WorkOrderStatusManager::canCloseWorkOrder(const QString &currentStatus)
{
    return currentStatus == OPEN || currentStatus == PROCESSING || currentStatus == REFUSED;
}

// 检查是否可以拒绝工单
bool WorkOrderStatusManager::canRefuseWorkOrder(const QString &currentStatus)
{
    return currentStatus == OPEN || currentStatus == PROCESSING;
}

// 检查是否可以开始处理工单
bool WorkOrderStatusManager::canStartProcessing(const QString &currentStatus)
{
    return currentStatus == OPEN;
}
