#ifndef BUSINESS_EXCEPTION_H
#define BUSINESS_EXCEPTION_H

#include <QString>
#include <QException>

// 业务异常基类
class BusinessException : public QException
{
public:
    explicit BusinessException(const QString& message, const QString& operation = QString());
    virtual ~BusinessException() = default;

    // 获取异常信息
    QString getMessage() const { return message_; }
    QString getOperation() const { return operation_; }
    
    // 获取完整错误信息
    QString getFullMessage() const;
    
    // QException接口
    void raise() const override { throw *this; }
    BusinessException* clone() const override { return new BusinessException(*this); }

protected:
    QString message_;
    QString operation_;
};

// 验证异常
class ValidationException : public BusinessException
{
public:
    explicit ValidationException(const QString& message, const QString& field = QString(), const QString& operation = QString());
    
    QString getField() const { return field_; }
    QString getFullMessage() const;

private:
    QString field_;
};

// 授权异常
class AuthorizationException : public BusinessException
{
public:
    explicit AuthorizationException(const QString& message, int userId = -1, const QString& operation = QString());
    
    int getUserId() const { return userId_; }
    QString getFullMessage() const;

private:
    int userId_;
};

// 资源不存在异常
class ResourceNotFoundException : public BusinessException
{
public:
    explicit ResourceNotFoundException(const QString& resourceType, const QString& resourceId, const QString& operation = QString());
    
    QString getResourceType() const { return resourceType_; }
    QString getResourceId() const { return resourceId_; }
    QString getFullMessage() const;

private:
    QString resourceType_;
    QString resourceId_;
};

// 状态转换异常
class StateTransitionException : public BusinessException
{
public:
    explicit StateTransitionException(const QString& currentState, const QString& targetState, const QString& operation = QString());
    
    QString getCurrentState() const { return currentState_; }
    QString getTargetState() const { return targetState_; }
    QString getFullMessage() const;

private:
    QString currentState_;
    QString targetState_;
};

#endif // BUSINESS_EXCEPTION_H
