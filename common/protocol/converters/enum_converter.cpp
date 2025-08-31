#include "enum_converter.h"

// 枚举转换函数实现
QString EnumConverter::workOrderStatusToString(WorkOrderStatus status) {
        switch (status) {
            case STATUS_CREATED: return "created";
            case STATUS_ASSIGNED: return "assigned";
            case STATUS_IN_PROGRESS: return "in_progress";
            case STATUS_RESOLVED: return "resolved";
            case STATUS_CLOSED: return "closed";
            default: return "unknown";
        }
    }
    
WorkOrderStatus EnumConverter::stringToWorkOrderStatus(const QString& str) {
        if (str == "created") return STATUS_CREATED;
        if (str == "assigned") return STATUS_ASSIGNED;
        if (str == "in_progress") return STATUS_IN_PROGRESS;
        if (str == "resolved") return STATUS_RESOLVED;
        if (str == "closed") return STATUS_CLOSED;
        return STATUS_CREATED; // 默认值
    }
    
QString EnumConverter::controlTypeToString(ControlType type) {
        switch (type) {
            case CONTROL_START_VIDEO: return "start_video";
            case CONTROL_STOP_VIDEO: return "stop_video";
            case CONTROL_START_AUDIO: return "start_audio";
            case CONTROL_STOP_AUDIO: return "stop_audio";
            case CONTROL_SCREENSHOT: return "screenshot";
            case CONTROL_DEVICE_CTRL: return "device_control";
            default: return "unknown";
        }
    }
    
ControlType EnumConverter::stringToControlType(const QString& str) {
        if (str == "start_video") return CONTROL_START_VIDEO;
        if (str == "stop_video") return CONTROL_STOP_VIDEO;
        if (str == "start_audio") return CONTROL_START_AUDIO;
        if (str == "stop_audio") return CONTROL_STOP_AUDIO;
        if (str == "screenshot") return CONTROL_SCREENSHOT;
        if (str == "device_control") return CONTROL_DEVICE_CTRL;
        return CONTROL_START_VIDEO; // 默认值
    }
    
QString EnumConverter::eventTypeToString(EventType type) {
        switch (type) {
            case EVENT_USER_JOINED: return "user_joined";
            case EVENT_USER_LEFT: return "user_left";
            case EVENT_ROOM_CREATED: return "room_created";
            case EVENT_ROOM_CLOSED: return "room_closed";
            case EVENT_WORKORDER_UPDATED: return "workorder_updated";
            default: return "unknown";
        }
    }
    
EventType EnumConverter::stringToEventType(const QString& str) {
        if (str == "user_joined") return EVENT_USER_JOINED;
        if (str == "user_left") return EVENT_USER_LEFT;
        if (str == "room_created") return EVENT_ROOM_CREATED;
        if (str == "room_closed") return EVENT_ROOM_CLOSED;
        if (str == "workorder_updated") return EVENT_WORKORDER_UPDATED;
        return EVENT_USER_JOINED; // 默认值
    }
