#include "business_logger.h"

// 基本日志方法
void BusinessLogger::error(const QString &operation, const QString &message)
{
    LOG_ERROR(LogModule::SYSTEM, LogLayer::BUSINESS, operation, message);
}

void BusinessLogger::warning(const QString &operation, const QString &message)
{
    LOG_WARNING(LogModule::SYSTEM, LogLayer::BUSINESS, operation, message);
}

void BusinessLogger::info(const QString &operation, const QString &message)
{
    LOG_INFO(LogModule::SYSTEM, LogLayer::BUSINESS, operation, message);
}

void BusinessLogger::debug(const QString &operation, const QString &message)
{
    LOG_DEBUG(LogModule::SYSTEM, LogLayer::BUSINESS, operation, message);
}

// 业务操作日志
void BusinessLogger::businessOperationStart(const QString &operation, const QString &context)
{
    QString message = formatBusinessContext(operation, context);
    LOG_INFO(LogModule::SYSTEM, LogLayer::BUSINESS, "Operation Start", message);
}

void BusinessLogger::businessOperationSuccess(const QString &operation, const QString &result)
{
    QString message = QString("Operation '%1' completed successfully").arg(operation);
    if (!result.isEmpty()) {
        message += QString(": %1").arg(result);
    }
    LOG_INFO(LogModule::SYSTEM, LogLayer::BUSINESS, "Operation Success", message);
}

void BusinessLogger::businessOperationFailed(const QString &operation, const QString &reason)
{
    QString message = QString("Operation '%1' failed").arg(operation);
    if (!reason.isEmpty()) {
        message += QString(": %1").arg(reason);
    }
    LOG_ERROR(LogModule::SYSTEM, LogLayer::BUSINESS, "Operation Failed", message);
}

// 用户相关日志
void BusinessLogger::userLogin(const QString &username, int userType, bool success, const QString &reason)
{
    QString userInfo = formatUserInfo(username, userType);
    if (success) {
        LOG_INFO(LogModule::USER, LogLayer::BUSINESS, "User Login", QString("User %1 logged in successfully").arg(userInfo));
    } else {
        QString message = QString("User %1 login failed").arg(userInfo);
        if (!reason.isEmpty()) {
            message += QString(": %1").arg(reason);
        }
        LOG_WARNING(LogModule::USER, LogLayer::BUSINESS, "User Login Failed", message);
    }
}

void BusinessLogger::userLogout(const QString &username)
{
    LOG_INFO(LogModule::USER, LogLayer::BUSINESS, "User Logout", QString("User '%1' logged out").arg(username));
}

void BusinessLogger::userRegistration(const QString &username, int userType, bool success, const QString &reason)
{
    QString userInfo = formatUserInfo(username, userType);
    if (success) {
        LOG_INFO(LogModule::USER, LogLayer::BUSINESS, "User Registration", QString("User %1 registered successfully").arg(userInfo));
    } else {
        QString message = QString("User %1 registration failed").arg(userInfo);
        if (!reason.isEmpty()) {
            message += QString(": %1").arg(reason);
        }
        LOG_ERROR(LogModule::USER, LogLayer::BUSINESS, "User Registration Failed", message);
    }
}

void BusinessLogger::userAuthentication(const QString &username, bool success, const QString &reason)
{
    if (success) {
        LOG_INFO(LogModule::USER, LogLayer::BUSINESS, "User Authentication", QString("User '%1' authenticated successfully").arg(username));
    } else {
        QString message = QString("User '%1' authentication failed").arg(username);
        if (!reason.isEmpty()) {
            message += QString(": %1").arg(reason);
        }
        LOG_WARNING(LogModule::USER, LogLayer::BUSINESS, "User Authentication Failed", message);
    }
}

// 工单相关日志
void BusinessLogger::workOrderCreated(const QString &ticketId, int creatorId, bool success, const QString &reason)
{
    QString workOrderInfo = formatWorkOrderInfo(ticketId, creatorId);
    if (success) {
        LOG_INFO(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Created", QString("Work order %1 created successfully").arg(workOrderInfo));
    } else {
        QString message = QString("Work order %1 creation failed").arg(workOrderInfo);
        if (!reason.isEmpty()) {
            message += QString(": %1").arg(reason);
        }
        LOG_ERROR(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Creation Failed", message);
    }
}

void BusinessLogger::workOrderUpdated(const QString &ticketId, const QString &changes, bool success, const QString &reason)
{
    QString workOrderInfo = formatWorkOrderInfo(ticketId, 0);
    if (success) {
        QString message = QString("Work order %1 updated successfully").arg(workOrderInfo);
        if (!changes.isEmpty()) {
            message += QString(": %1").arg(changes);
        }
        LOG_INFO(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Updated", message);
    } else {
        QString message = QString("Work order %1 update failed").arg(workOrderInfo);
        if (!reason.isEmpty()) {
            message += QString(": %1").arg(reason);
        }
        LOG_ERROR(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Update Failed", message);
    }
}

void BusinessLogger::workOrderStatusChanged(const QString &ticketId, const QString &oldStatus, const QString &newStatus, bool success)
{
    QString workOrderInfo = formatWorkOrderInfo(ticketId, 0);
    if (success) {
        LOG_INFO(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Status Changed", 
                QString("Work order %1 status changed from '%2' to '%3'").arg(workOrderInfo).arg(oldStatus).arg(newStatus));
    } else {
        LOG_ERROR(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Status Change Failed", 
                 QString("Failed to change work order %1 status from '%2' to '%3'").arg(workOrderInfo).arg(oldStatus).arg(newStatus));
    }
}

void BusinessLogger::workOrderAssigned(const QString &ticketId, int assigneeId, bool success, const QString &reason)
{
    QString workOrderInfo = formatWorkOrderInfo(ticketId, assigneeId);
    if (success) {
        LOG_INFO(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Assigned", 
                QString("Work order %1 assigned successfully").arg(workOrderInfo));
    } else {
        QString message = QString("Work order %1 assignment failed").arg(workOrderInfo);
        if (!reason.isEmpty()) {
            message += QString(": %1").arg(reason);
        }
        LOG_ERROR(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Assignment Failed", message);
    }
}

void BusinessLogger::workOrderClosed(const QString &ticketId, int userId, bool success, const QString &reason)
{
    QString workOrderInfo = formatWorkOrderInfo(ticketId, userId);
    if (success) {
        LOG_INFO(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Closed", 
                QString("Work order %1 closed successfully").arg(workOrderInfo));
    } else {
        QString message = QString("Work order %1 close failed").arg(workOrderInfo);
        if (!reason.isEmpty()) {
            message += QString(": %1").arg(reason);
        }
        LOG_ERROR(LogModule::WORKORDER, LogLayer::BUSINESS, "Work Order Close Failed", message);
    }
}

// 会话相关日志
void BusinessLogger::sessionCreated(const QString &sessionId, int userId, const QString &roomId)
{
    QString sessionInfo = formatSessionInfo(sessionId, userId, roomId);
    LOG_INFO(LogModule::SYSTEM, LogLayer::BUSINESS, "Session Created", QString("Session %1 created").arg(sessionInfo));
}

void BusinessLogger::sessionJoined(const QString &sessionId, int userId, const QString &roomId)
{
    QString sessionInfo = formatSessionInfo(sessionId, userId, roomId);
    LOG_INFO(LogModule::SYSTEM, LogLayer::BUSINESS, "Session Joined", QString("User joined session %1").arg(sessionInfo));
}

void BusinessLogger::sessionLeft(const QString &sessionId, int userId, const QString &roomId)
{
    QString sessionInfo = formatSessionInfo(sessionId, userId, roomId);
    LOG_INFO(LogModule::SYSTEM, LogLayer::BUSINESS, "Session Left", QString("User left session %1").arg(sessionInfo));
}

void BusinessLogger::sessionExpired(const QString &sessionId, int userId)
{
    LOG_WARNING(LogModule::SYSTEM, LogLayer::BUSINESS, "Session Expired", 
               QString("Session '%1' for user %2 expired").arg(sessionId).arg(userId));
}

// 权限相关日志
void BusinessLogger::permissionCheck(const QString &operation, int userId, bool granted, const QString &reason)
{
    if (granted) {
        LOG_DEBUG(LogModule::SYSTEM, LogLayer::BUSINESS, "Permission Granted", 
                 QString("User %1 granted permission for operation '%2'").arg(userId).arg(operation));
    } else {
        QString message = QString("User %1 denied permission for operation '%2'").arg(userId).arg(operation);
        if (!reason.isEmpty()) {
            message += QString(": %1").arg(reason);
        }
        LOG_WARNING(LogModule::SYSTEM, LogLayer::BUSINESS, "Permission Denied", message);
    }
}

void BusinessLogger::authorizationFailed(const QString &operation, int userId, const QString &reason)
{
    QString message = QString("Authorization failed for user %1 on operation '%2'").arg(userId).arg(operation);
    if (!reason.isEmpty()) {
        message += QString(": %1").arg(reason);
    }
    LOG_ERROR(LogModule::SYSTEM, LogLayer::BUSINESS, "Authorization Failed", message);
}

// 验证相关日志
void BusinessLogger::validationFailed(const QString &operation, const QString &field, const QString &reason)
{
    LOG_WARNING(LogModule::SYSTEM, LogLayer::BUSINESS, "Validation Failed", 
               QString("Operation '%1' validation failed for field '%2': %3").arg(operation).arg(field).arg(reason));
}

void BusinessLogger::validationSuccess(const QString &operation, const QString &context)
{
    QString message = QString("Operation '%1' validation successful").arg(operation);
    if (!context.isEmpty()) {
        message += QString(": %1").arg(context);
    }
    LOG_DEBUG(LogModule::SYSTEM, LogLayer::BUSINESS, "Validation Success", message);
}

// 事件相关日志
void BusinessLogger::eventTriggered(const QString &eventType, const QString &context, const QJsonObject &data)
{
    QString message = QString("Event '%1' triggered in context '%2'").arg(eventType).arg(context);
    if (!data.isEmpty()) {
        message += QString(" with data: %1").arg(QString::fromUtf8(QJsonDocument(data).toJson()));
    }
    LOG_INFO(LogModule::SYSTEM, LogLayer::BUSINESS, "Event Triggered", message);
}

void BusinessLogger::eventHandled(const QString &eventType, bool success, const QString &result)
{
    if (success) {
        QString message = QString("Event '%1' handled successfully").arg(eventType);
        if (!result.isEmpty()) {
            message += QString(": %1").arg(result);
        }
        LOG_INFO(LogModule::SYSTEM, LogLayer::BUSINESS, "Event Handled", message);
    } else {
        LOG_ERROR(LogModule::SYSTEM, LogLayer::BUSINESS, "Event Handling Failed", 
                 QString("Failed to handle event '%1': %2").arg(eventType).arg(result));
    }
}

// 私有辅助方法
QString BusinessLogger::formatBusinessContext(const QString &operation, const QString &context)
{
    QString message = QString("Operation: %1").arg(operation);
    if (!context.isEmpty()) {
        message += QString(", Context: %1").arg(context);
    }
    return message;
}

QString BusinessLogger::formatUserInfo(const QString &username, int userType)
{
    QString userTypeStr = (userType == 0) ? "Factory" : "Expert";
    return QString("'%1' (%2)").arg(username).arg(userTypeStr);
}

QString BusinessLogger::formatWorkOrderInfo(const QString &ticketId, int userId)
{
    QString info = QString("'%1'").arg(ticketId);
    if (userId > 0) {
        info += QString(" (User: %1)").arg(userId);
    }
    return info;
}

QString BusinessLogger::formatSessionInfo(const QString &sessionId, int userId, const QString &roomId)
{
    QString info = QString("'%1' (User: %2)").arg(sessionId).arg(userId);
    if (!roomId.isEmpty()) {
        info += QString(" (Room: %1)").arg(roomId);
    }
    return info;
}
