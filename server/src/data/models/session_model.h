#ifndef SESSION_MODEL_H
#define SESSION_MODEL_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

struct SessionModel {
    int id = -1;
    QString sessionId;
    int userId = -1;
    QString roomId;
    QString status;
    QDateTime createdAt;
    QDateTime lastActivity;
    QDateTime expiresAt;
    
    // 辅助方法
    bool isValid() const;
    QJsonObject toJson() const;
    static SessionModel fromJson(const QJsonObject& json);
    
    // 状态常量
    static const QString STATUS_ACTIVE;
    static const QString STATUS_INACTIVE;
    static const QString STATUS_EXPIRED;
};

#endif // SESSION_MODEL_H
