#ifndef BUSINESS_LOGGER_H
#define BUSINESS_LOGGER_H

#include "../../logging/managers/log_manager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// 业务逻辑层日志助手类
class BusinessLogger
{
public:
    // 基本日志方法
    static void error(const QString &operation, const QString &message);
    static void warning(const QString &operation, const QString &message);
    static void info(const QString &operation, const QString &message);
    static void debug(const QString &operation, const QString &message);
    
    // 业务操作日志
    static void businessOperationStart(const QString &operation, const QString &context = QString());
    static void businessOperationSuccess(const QString &operation, const QString &result = QString());
    static void businessOperationFailed(const QString &operation, const QString &reason = QString());
    
    // 用户相关日志
    static void userLogin(const QString &username, int userType, bool success, const QString &reason = QString());
    static void userLogout(const QString &username);
    static void userRegistration(const QString &username, int userType, bool success, const QString &reason = QString());
    static void userAuthentication(const QString &username, bool success, const QString &reason = QString());
    
    // 工单相关日志
    static void workOrderCreated(const QString &ticketId, int creatorId, bool success, const QString &reason = QString());
    static void workOrderUpdated(const QString &ticketId, const QString &changes, bool success, const QString &reason = QString());
    static void workOrderStatusChanged(const QString &ticketId, const QString &oldStatus, const QString &newStatus, bool success);
    static void workOrderAssigned(const QString &ticketId, int assigneeId, bool success, const QString &reason = QString());
    static void workOrderClosed(const QString &ticketId, int userId, bool success, const QString &reason = QString());
    
    // 会话相关日志
    static void sessionCreated(const QString &sessionId, int userId, const QString &roomId);
    static void sessionJoined(const QString &sessionId, int userId, const QString &roomId);
    static void sessionLeft(const QString &sessionId, int userId, const QString &roomId);
    static void sessionExpired(const QString &sessionId, int userId);
    
    // 权限相关日志
    static void permissionCheck(const QString &operation, int userId, bool granted, const QString &reason = QString());
    static void authorizationFailed(const QString &operation, int userId, const QString &reason);
    
    // 验证相关日志
    static void validationFailed(const QString &operation, const QString &field, const QString &reason);
    static void validationSuccess(const QString &operation, const QString &context = QString());
    
    // 事件相关日志
    static void eventTriggered(const QString &eventType, const QString &context, const QJsonObject &data = QJsonObject());
    static void eventHandled(const QString &eventType, bool success, const QString &result = QString());

private:
    // 格式化业务上下文信息
    static QString formatBusinessContext(const QString &operation, const QString &context);
    
    // 格式化用户信息
    static QString formatUserInfo(const QString &username, int userType);
    
    // 格式化工单信息
    static QString formatWorkOrderInfo(const QString &ticketId, int userId);
    
    // 格式化会话信息
    static QString formatSessionInfo(const QString &sessionId, int userId, const QString &roomId);
};

#endif // BUSINESS_LOGGER_H
