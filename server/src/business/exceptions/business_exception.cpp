#include "business_exception.h"

// BusinessException实现
BusinessException::BusinessException(const QString& message, const QString& operation)
    : message_(message), operation_(operation)
{
}

QString BusinessException::getFullMessage() const
{
    QString fullMessage = message_;
    if (!operation_.isEmpty()) {
        fullMessage = QString("Operation '%1': %2").arg(operation_).arg(message_);
    }
    return fullMessage;
}

// ValidationException实现
ValidationException::ValidationException(const QString& message, const QString& field, const QString& operation)
    : BusinessException(message, operation), field_(field)
{
}

QString ValidationException::getFullMessage() const
{
    QString fullMessage = BusinessException::getFullMessage();
    if (!field_.isEmpty()) {
        fullMessage = QString("Field '%1': %2").arg(field_).arg(fullMessage);
    }
    return fullMessage;
}

// AuthorizationException实现
AuthorizationException::AuthorizationException(const QString& message, int userId, const QString& operation)
    : BusinessException(message, operation), userId_(userId)
{
}

QString AuthorizationException::getFullMessage() const
{
    QString fullMessage = BusinessException::getFullMessage();
    if (userId_ > 0) {
        fullMessage = QString("User %1: %2").arg(userId_).arg(fullMessage);
    }
    return fullMessage;
}

// ResourceNotFoundException实现
ResourceNotFoundException::ResourceNotFoundException(const QString& resourceType, const QString& resourceId, const QString& operation)
    : BusinessException(QString("Resource not found"), operation), resourceType_(resourceType), resourceId_(resourceId)
{
}

QString ResourceNotFoundException::getFullMessage() const
{
    QString fullMessage = BusinessException::getFullMessage();
    if (!resourceType_.isEmpty() && !resourceId_.isEmpty()) {
        fullMessage = QString("%1 '%2' not found").arg(resourceType_).arg(resourceId_);
    }
    return fullMessage;
}

// StateTransitionException实现
StateTransitionException::StateTransitionException(const QString& currentState, const QString& targetState, const QString& operation)
    : BusinessException(QString("Invalid state transition"), operation), currentState_(currentState), targetState_(targetState)
{
}

QString StateTransitionException::getFullMessage() const
{
    QString fullMessage = BusinessException::getFullMessage();
    if (!currentState_.isEmpty() && !targetState_.isEmpty()) {
        fullMessage = QString("Cannot transition from '%1' to '%2'").arg(currentState_).arg(targetState_);
    }
    return fullMessage;
}
