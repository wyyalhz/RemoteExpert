#ifndef WORKORDER_STATUS_MANAGER_H
#define WORKORDER_STATUS_MANAGER_H

#include <QString>
#include <QStringList>
#include <QObject>

// 工单状态管理器
class WorkOrderStatusManager : public QObject
{
    Q_OBJECT
public:
    // 工单状态定义
    static const QString OPEN;
    static const QString PROCESSING;
    static const QString REFUSED;
    static const QString CLOSED;
    
    // 状态等级映射
    static int getStatusLevel(const QString &status);
    
    // 状态转换合法性验证
    static bool isValidTransition(const QString &fromStatus, const QString &toStatus);
    
    // 获取所有有效状态
    static QStringList getValidStatuses();
    
    // 验证状态是否有效
    static bool isValidStatus(const QString &status);
    
    // 获取状态描述
    static QString getStatusDescription(const QString &status);
    
    // 获取下一个可能的状态列表
    static QStringList getNextPossibleStatuses(const QString &currentStatus);
    
    // 检查是否可以关闭工单
    static bool canCloseWorkOrder(const QString &currentStatus);
    
    // 检查是否可以拒绝工单
    static bool canRefuseWorkOrder(const QString &currentStatus);
    
    // 检查是否可以开始处理工单
    static bool canStartProcessing(const QString &currentStatus);

private:
    // 私有构造函数，防止实例化
    WorkOrderStatusManager() = delete;
};

#endif // WORKORDER_STATUS_MANAGER_H
