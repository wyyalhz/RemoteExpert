#ifndef SESSION_SERVICE_H
#define SESSION_SERVICE_H

#include "../exceptions/business_exception.h"
#include "../logging/business_logger.h"
#include "../../data/databasemanager.h"
#include "../../data/models/session_model.h"
#include "../../data/repositories/session_repository.h"
#include <QObject>
#include <QJsonObject>
#include <QTimer>

// 会话业务服务
class SessionService : public QObject
{
    Q_OBJECT
public:
    explicit SessionService(DatabaseManager* dbManager, QObject *parent = nullptr);
    ~SessionService();

    // 会话生命周期管理
    QString createSession(int userId, const QString& roomId, int timeoutMinutes = 120);
    bool updateSessionActivity(const QString& sessionId);
    bool expireSession(const QString& sessionId);
    bool cleanupExpiredSessions();
    
    // 会话查询
    SessionModel getSession(const QString& sessionId);
    QList<SessionModel> getUserSessions(int userId);
    QList<SessionModel> getRoomSessions(const QString& roomId);
    QList<SessionModel> getActiveSessions();
    QList<SessionModel> getExpiredSessions();
    
    // 会话验证
    bool isSessionValid(const QString& sessionId);
    bool isUserInRoom(int userId, const QString& roomId);
    bool isSessionExpired(const QString& sessionId);
    
    // 会话统计
    int getActiveSessionCount();
    int getUserSessionCount(int userId);
    int getRoomSessionCount(const QString& roomId);
    
    // 会话管理
    bool removeSession(const QString& sessionId);
    bool removeUserSessions(int userId);
    bool removeRoomSessions(const QString& roomId);
    
    // 会话超时设置
    void setSessionTimeout(int minutes);
    int getSessionTimeout() const;

private slots:
    void onCleanupTimer();

private:
    DatabaseManager* dbManager_;
    SessionRepository* sessionRepo_;
    QTimer* cleanupTimer_;
    int sessionTimeoutMinutes_;
    
    // 私有辅助方法
    QString generateSessionId();
    QDateTime calculateExpiryTime(int timeoutMinutes);
    void logSessionActivity(const QString& operation, const QString& sessionId, bool success, const QString& reason = QString());
    
    // 业务验证方法
    bool validateSessionCreation(int userId, const QString& roomId);
    bool validateSessionAccess(const QString& sessionId, int userId);
    
    // 业务事件触发方法
    void triggerSessionCreatedEvent(const SessionModel& session);
    void triggerSessionExpiredEvent(const SessionModel& session);
    void triggerSessionActivityEvent(const SessionModel& session);
};

#endif // SESSION_SERVICE_H
