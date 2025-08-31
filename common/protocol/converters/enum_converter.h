#pragma once
// ===============================================
// common/protocol/converters/enum_converter.h
// 枚举转换工具
// ===============================================

#include <QString>
#include "../types/enums.h"

// 枚举转换工具类
class EnumConverter {
public:
    // 工单状态转换
    static QString workOrderStatusToString(WorkOrderStatus status);
    static WorkOrderStatus stringToWorkOrderStatus(const QString& str);
    
    // 控制类型转换
    static QString controlTypeToString(ControlType type);
    static ControlType stringToControlType(const QString& str);
    
    // 事件类型转换
    static QString eventTypeToString(EventType type);
    static EventType stringToEventType(const QString& str);
};
