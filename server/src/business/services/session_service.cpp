#include "session_service.h"
#include <QUuid>
#include <QDateTime>
#include <QTimer>

SessionService::SessionService(DatabaseManager* dbManager, QObject *parent)
    : QObject(parent)
    , dbManager_(dbManager)
    , sessionRepo_(dbManager->sessionRepository())
    , cleanupTimer_(new QTimer(this))
    , sessionTimeoutMinutes_(120)
{
    // 设置定时清理过期会话（每5分钟执行一次）
    connect(cleanupTimer_, &QTimer::timeout, this, &SessionService::onCleanupTimer);
    cleanupTimer_->start(5 * 60 * 1000); // 5分钟
    
    BusinessLogger::info("Session Service", "Session service initialized");
}

SessionService::~SessionService()
{
    if (cleanupTimer_) {
        cleanupTimer_->stop();
    }
    BusinessLogger::info("Session Service", "Session service destroyed");
}

// 会话生命周期管理
QString SessionService::createSession(int userId, const QString& roomId, int timeoutMinutes)
{
    BusinessLogger::businessOperationStart("Create Session", QString("User: %1, Room: %2").arg(userId).arg(roomId));
    
    try {
        // 业务验证
        if (!validateSessionCreation(userId, roomId)) {
            BusinessLogger::businessOperationFailed("Create Session", "Validation failed");
            return QString();
        }
        
        // 生成会话ID
        QString sessionId = generateSessionId();
        
        // 创建会话模型
        SessionModel session;
        session.sessionId = sessionId;
        session.userId = userId;
        session.roomId = roomId;
        session.status = SessionModel::STATUS_ACTIVE;
        session.createdAt = QDateTime::currentDateTime();
        session.lastActivity = QDateTime::currentDateTime();
        session.expiresAt = calculateExpiryTime(timeoutMinutes);
        
        // 保存到数据库
        int dbSessionId = -1;
        bool success = sessionRepo_->create(session, dbSessionId);
        
        if (success) {
            session.id = dbSessionId;
            triggerSessionCreatedEvent(session);
            logSessionActivity("Create Session", sessionId, true);
            BusinessLogger::businessOperationSuccess("Create Session", sessionId);
            return sessionId;
        } else {
            logSessionActivity("Create Session", sessionId, false, "Database operation failed");
            BusinessLogger::businessOperationFailed("Create Session", "Database operation failed");
            return QString();
        }
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Create Session", e.getMessage());
        return QString();
    }
}

bool SessionService::updateSessionActivity(const QString& sessionId)
{
    BusinessLogger::businessOperationStart("Update Session Activity", sessionId);
    
    try {
        // 获取会话信息
        SessionModel session = getSession(sessionId);
        if (!session.isValid()) {
            BusinessLogger::businessOperationFailed("Update Session Activity", "Session not found");
            return false;
        }
        
        // 检查会话是否已过期
        if (isSessionExpired(sessionId)) {
            expireSession(sessionId);
            BusinessLogger::businessOperationFailed("Update Session Activity", "Session expired");
            return false;
        }
        
        // 更新最后活动时间
        bool success = sessionRepo_->updateLastActivity(session.id);
        
        if (success) {
            triggerSessionActivityEvent(session);
            logSessionActivity("Update Activity", sessionId, true);
            BusinessLogger::businessOperationSuccess("Update Session Activity", sessionId);
        } else {
            logSessionActivity("Update Activity", sessionId, false, "Database operation failed");
            BusinessLogger::businessOperationFailed("Update Session Activity", "Database operation failed");
        }
        
        return success;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Update Session Activity", e.getMessage());
        return false;
    }
}

bool SessionService::expireSession(const QString& sessionId)
{
    BusinessLogger::businessOperationStart("Expire Session", sessionId);
    
    try {
        SessionModel session = getSession(sessionId);
        if (!session.isValid()) {
            BusinessLogger::businessOperationFailed("Expire Session", "Session not found");
            return false;
        }
        
        bool success = sessionRepo_->expireSession(session.id);
        
        if (success) {
            triggerSessionExpiredEvent(session);
            logSessionActivity("Expire Session", sessionId, true);
            BusinessLogger::businessOperationSuccess("Expire Session", sessionId);
        } else {
            logSessionActivity("Expire Session", sessionId, false, "Database operation failed");
            BusinessLogger::businessOperationFailed("Expire Session", "Database operation failed");
        }
        
        return success;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Expire Session", e.getMessage());
        return false;
    }
}

bool SessionService::cleanupExpiredSessions()
{
    BusinessLogger::businessOperationStart("Cleanup Expired Sessions", "");
    
    try {
        QList<SessionModel> expiredSessions = getExpiredSessions();
        int cleanedCount = 0;
        
        for (const SessionModel& session : expiredSessions) {
            if (removeSession(session.sessionId)) {
                cleanedCount++;
            }
        }
        
        BusinessLogger::businessOperationSuccess("Cleanup Expired Sessions", 
                                               QString("Cleaned %1 expired sessions").arg(cleanedCount));
        return true;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Cleanup Expired Sessions", e.getMessage());
        return false;
    }
}

// 会话查询
SessionModel SessionService::getSession(const QString& sessionId)
{
    SessionModel session;
    if (sessionRepo_->findBySessionId(sessionId, session)) {
        return session;
    }
    return SessionModel();
}

QList<SessionModel> SessionService::getUserSessions(int userId)
{
    return sessionRepo_->findByUserId(userId);
}

QList<SessionModel> SessionService::getRoomSessions(const QString& roomId)
{
    return sessionRepo_->findByRoomId(roomId);
}

QList<SessionModel> SessionService::getActiveSessions()
{
    return sessionRepo_->findByStatus(SessionModel::STATUS_ACTIVE);
}

QList<SessionModel> SessionService::getExpiredSessions()
{
    return sessionRepo_->findExpiredSessions();
}

// 会话验证
bool SessionService::isSessionValid(const QString& sessionId)
{
    SessionModel session = getSession(sessionId);
    if (!session.isValid()) {
        return false;
    }
    
    return session.status == SessionModel::STATUS_ACTIVE && !isSessionExpired(sessionId);
}

bool SessionService::isUserInRoom(int userId, const QString& roomId)
{
    QList<SessionModel> sessions = getUserSessions(userId);
    for (const SessionModel& session : sessions) {
        if (session.roomId == roomId && session.status == SessionModel::STATUS_ACTIVE) {
            return !isSessionExpired(session.sessionId);
        }
    }
    return false;
}

bool SessionService::isSessionExpired(const QString& sessionId)
{
    SessionModel session = getSession(sessionId);
    if (!session.isValid()) {
        return true;
    }
    
    QDateTime currentTime = QDateTime::currentDateTime();
    
    // 检查过期时间
    if (session.expiresAt.isValid() && currentTime > session.expiresAt) {
        return true;
    }
    
    // 检查最后活动时间（超过超时时间视为过期）
    QDateTime timeoutTime = session.lastActivity.addSecs(sessionTimeoutMinutes_ * 60);
    return currentTime > timeoutTime;
}

// 会话统计
int SessionService::getActiveSessionCount()
{
    return sessionRepo_->countByStatus(SessionModel::STATUS_ACTIVE);
}

int SessionService::getUserSessionCount(int userId)
{
    return sessionRepo_->countByUserId(userId);
}

int SessionService::getRoomSessionCount(const QString& roomId)
{
    return sessionRepo_->countByRoomId(roomId);
}

// 会话管理
bool SessionService::removeSession(const QString& sessionId)
{
    BusinessLogger::businessOperationStart("Remove Session", sessionId);
    
    try {
        SessionModel session = getSession(sessionId);
        if (!session.isValid()) {
            BusinessLogger::businessOperationFailed("Remove Session", "Session not found");
            return false;
        }
        
        bool success = sessionRepo_->remove(session.id);
        
        if (success) {
            logSessionActivity("Remove Session", sessionId, true);
            BusinessLogger::businessOperationSuccess("Remove Session", sessionId);
        } else {
            logSessionActivity("Remove Session", sessionId, false, "Database operation failed");
            BusinessLogger::businessOperationFailed("Remove Session", "Database operation failed");
        }
        
        return success;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Remove Session", e.getMessage());
        return false;
    }
}

bool SessionService::removeUserSessions(int userId)
{
    BusinessLogger::businessOperationStart("Remove User Sessions", QString::number(userId));
    
    try {
        QList<SessionModel> sessions = getUserSessions(userId);
        int removedCount = 0;
        
        for (const SessionModel& session : sessions) {
            if (removeSession(session.sessionId)) {
                removedCount++;
            }
        }
        
        BusinessLogger::businessOperationSuccess("Remove User Sessions", 
                                               QString("Removed %1 sessions for user %2").arg(removedCount).arg(userId));
        return true;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Remove User Sessions", e.getMessage());
        return false;
    }
}

bool SessionService::removeRoomSessions(const QString& roomId)
{
    BusinessLogger::businessOperationStart("Remove Room Sessions", roomId);
    
    try {
        QList<SessionModel> sessions = getRoomSessions(roomId);
        int removedCount = 0;
        
        for (const SessionModel& session : sessions) {
            if (removeSession(session.sessionId)) {
                removedCount++;
            }
        }
        
        BusinessLogger::businessOperationSuccess("Remove Room Sessions", 
                                               QString("Removed %1 sessions for room %2").arg(removedCount).arg(roomId));
        return true;
    }
    catch (const BusinessException& e) {
        BusinessLogger::businessOperationFailed("Remove Room Sessions", e.getMessage());
        return false;
    }
}

// 会话超时设置
void SessionService::setSessionTimeout(int minutes)
{
    sessionTimeoutMinutes_ = minutes;
    BusinessLogger::info("Session Service", QString("Session timeout set to %1 minutes").arg(minutes));
}

int SessionService::getSessionTimeout() const
{
    return sessionTimeoutMinutes_;
}

// 私有槽函数
void SessionService::onCleanupTimer()
{
    cleanupExpiredSessions();
}

// 私有辅助方法
QString SessionService::generateSessionId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QDateTime SessionService::calculateExpiryTime(int timeoutMinutes)
{
    return QDateTime::currentDateTime().addSecs(timeoutMinutes * 60);
}

void SessionService::logSessionActivity(const QString& operation, const QString& sessionId, bool success, const QString& reason)
{
    if (success) {
        BusinessLogger::info("Session Service", QString("%1: %2").arg(operation).arg(sessionId));
    } else {
        BusinessLogger::error("Session Service", QString("%1 failed: %2 - %3").arg(operation).arg(sessionId).arg(reason));
    }
}

// 业务验证方法
bool SessionService::validateSessionCreation(int userId, const QString& roomId)
{
    if (userId <= 0) {
        BusinessLogger::warning("Session Service", "Invalid user ID for session creation");
        return false;
    }
    
    if (roomId.isEmpty()) {
        BusinessLogger::warning("Session Service", "Empty room ID for session creation");
        return false;
    }
    
    // 检查用户是否已在房间中
    if (isUserInRoom(userId, roomId)) {
        BusinessLogger::warning("Session Service", QString("User %1 already in room %2").arg(userId).arg(roomId));
        return false;
    }
    
    return true;
}

bool SessionService::validateSessionAccess(const QString& sessionId, int userId)
{
    SessionModel session = getSession(sessionId);
    if (!session.isValid()) {
        return false;
    }
    
    return session.userId == userId;
}

// 业务事件触发方法
void SessionService::triggerSessionCreatedEvent(const SessionModel& session)
{
    BusinessLogger::info("Session Service", QString("Session created event: %1 for user %2 in room %3")
                        .arg(session.sessionId).arg(session.userId).arg(session.roomId));
}

void SessionService::triggerSessionExpiredEvent(const SessionModel& session)
{
    BusinessLogger::info("Session Service", QString("Session expired event: %1 for user %2 in room %3")
                        .arg(session.sessionId).arg(session.userId).arg(session.roomId));
}

void SessionService::triggerSessionActivityEvent(const SessionModel& session)
{
    BusinessLogger::debug("Session Service", QString("Session activity event: %1 for user %2 in room %3")
                         .arg(session.sessionId).arg(session.userId).arg(session.roomId));
}
