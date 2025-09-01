#include "session_repository.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDateTime>
#include <QUuid>
#include "../logging/db_logger.h"

SessionRepository::SessionRepository(QObject *parent) : DBBase(parent) {}

SessionRepository::~SessionRepository() {}

// 基础CRUD操作
bool SessionRepository::create(const SessionModel& session, int& sessionId)
{
    if (!checkConnection("Create Session")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        INSERT INTO sessions (session_id, user_id, room_id, status, created_at, last_activity, expires_at)
        VALUES (:session_id, :user_id, :room_id, :status, :created_at, :last_activity, :expires_at)
    )");

    query.bindValue(":session_id", session.sessionId);
    query.bindValue(":user_id", session.userId);
    query.bindValue(":room_id", session.roomId);
    query.bindValue(":status", session.status);
    query.bindValue(":created_at", session.createdAt);
    query.bindValue(":last_activity", session.lastActivity);
    query.bindValue(":expires_at", session.expiresAt);

    if (!executeQuery(query, "Create Session")) {
        return false;
    }

    sessionId = query.lastInsertId().toInt();
    DBLogger::info("Session Repository", QString("Session created with ID: %1").arg(sessionId));
    return true;
}

bool SessionRepository::findById(int sessionId, SessionModel& session)
{
    if (!checkConnection("Find Session By ID")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM sessions WHERE id = :id");
    query.bindValue(":id", sessionId);

    if (!executeQuery(query, "Find Session By ID")) {
        return false;
    }

    if (query.next()) {
        session = mapToModel(query.record());
        return true;
    }

    return false;
}

bool SessionRepository::findBySessionId(const QString& sessionId, SessionModel& session)
{
    if (!checkConnection("Find Session By Session ID")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM sessions WHERE session_id = :session_id");
    query.bindValue(":session_id", sessionId);

    if (!executeQuery(query, "Find Session By Session ID")) {
        return false;
    }

    if (query.next()) {
        session = mapToModel(query.record());
        return true;
    }

    return false;
}

bool SessionRepository::update(const SessionModel& session)
{
    if (!checkConnection("Update Session")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE sessions 
        SET user_id = :user_id, room_id = :room_id, status = :status, 
            last_activity = :last_activity, expires_at = :expires_at
        WHERE id = :id
    )");

    query.bindValue(":id", session.id);
    query.bindValue(":user_id", session.userId);
    query.bindValue(":room_id", session.roomId);
    query.bindValue(":status", session.status);
    query.bindValue(":last_activity", session.lastActivity);
    query.bindValue(":expires_at", session.expiresAt);

    if (!executeQuery(query, "Update Session")) {
        return false;
    }

    DBLogger::info("Session Repository", QString("Session updated: %1").arg(session.id));
    return true;
}

bool SessionRepository::remove(int sessionId)
{
    if (!checkConnection("Remove Session")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("DELETE FROM sessions WHERE id = :id");
    query.bindValue(":id", sessionId);

    if (!executeQuery(query, "Remove Session")) {
        return false;
    }

    DBLogger::info("Session Repository", QString("Session removed: %1").arg(sessionId));
    return true;
}

// 会话管理
bool SessionRepository::updateLastActivity(int sessionId)
{
    if (!checkConnection("Update Session Last Activity")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("UPDATE sessions SET last_activity = :last_activity WHERE id = :id");
    query.bindValue(":id", sessionId);
    query.bindValue(":last_activity", QDateTime::currentDateTime());

    if (!executeQuery(query, "Update Session Last Activity")) {
        return false;
    }

    return true;
}

bool SessionRepository::updateStatus(int sessionId, const QString& status)
{
    if (!checkConnection("Update Session Status")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("UPDATE sessions SET status = :status WHERE id = :id");
    query.bindValue(":id", sessionId);
    query.bindValue(":status", status);

    if (!executeQuery(query, "Update Session Status")) {
        return false;
    }

    DBLogger::info("Session Repository", QString("Session status updated: %1 -> %2").arg(sessionId).arg(status));
    return true;
}

bool SessionRepository::expireSession(int sessionId)
{
    return updateStatus(sessionId, SessionModel::STATUS_EXPIRED);
}

// 查询操作
QList<SessionModel> SessionRepository::findByUserId(int userId)
{
    QList<SessionModel> sessions;
    
    if (!checkConnection("Find Sessions By User ID")) {
        return sessions;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM sessions WHERE user_id = :user_id ORDER BY last_activity DESC");
    query.bindValue(":user_id", userId);

    if (!executeQuery(query, "Find Sessions By User ID")) {
        return sessions;
    }

    while (query.next()) {
        sessions.append(mapToModel(query.record()));
    }

    return sessions;
}

QList<SessionModel> SessionRepository::findByRoomId(const QString& roomId)
{
    QList<SessionModel> sessions;
    
    if (!checkConnection("Find Sessions By Room ID")) {
        return sessions;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM sessions WHERE room_id = :room_id AND status = :status ORDER BY last_activity DESC");
    query.bindValue(":room_id", roomId);
    query.bindValue(":status", SessionModel::STATUS_ACTIVE);

    if (!executeQuery(query, "Find Sessions By Room ID")) {
        return sessions;
    }

    while (query.next()) {
        sessions.append(mapToModel(query.record()));
    }

    return sessions;
}

QList<SessionModel> SessionRepository::findByStatus(const QString& status)
{
    QList<SessionModel> sessions;
    
    if (!checkConnection("Find Sessions By Status")) {
        return sessions;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM sessions WHERE status = :status ORDER BY last_activity DESC");
    query.bindValue(":status", status);

    if (!executeQuery(query, "Find Sessions By Status")) {
        return sessions;
    }

    while (query.next()) {
        sessions.append(mapToModel(query.record()));
    }

    return sessions;
}

QList<SessionModel> SessionRepository::findExpiredSessions()
{
    QList<SessionModel> sessions;
    
    if (!checkConnection("Find Expired Sessions")) {
        return sessions;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        SELECT * FROM sessions 
        WHERE (expires_at IS NOT NULL AND expires_at < :current_time) 
           OR (last_activity < :inactive_time AND status = :active_status)
        ORDER BY last_activity ASC
    )");
    
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime inactiveTime = currentTime.addSecs(-3600); // 1小时无活动视为过期
    
    query.bindValue(":current_time", currentTime);
    query.bindValue(":inactive_time", inactiveTime);
    query.bindValue(":active_status", SessionModel::STATUS_ACTIVE);

    if (!executeQuery(query, "Find Expired Sessions")) {
        return sessions;
    }

    while (query.next()) {
        sessions.append(mapToModel(query.record()));
    }

    return sessions;
}

QList<SessionModel> SessionRepository::findAll(int limit, int offset)
{
    QList<SessionModel> sessions;
    
    if (!checkConnection("Find All Sessions")) {
        return sessions;
    }

    QString sql = "SELECT * FROM sessions ORDER BY last_activity DESC";
    if (limit > 0) {
        sql += QString(" LIMIT %1 OFFSET %2").arg(limit).arg(offset);
    }

    QSqlQuery query(database());
    query.prepare(sql);

    if (!executeQuery(query, "Find All Sessions")) {
        return sessions;
    }

    while (query.next()) {
        sessions.append(mapToModel(query.record()));
    }

    return sessions;
}

// 统计查询
int SessionRepository::countByUserId(int userId)
{
    if (!checkConnection("Count Sessions By User ID")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM sessions WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!executeQuery(query, "Count Sessions By User ID")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int SessionRepository::countByRoomId(const QString& roomId)
{
    if (!checkConnection("Count Sessions By Room ID")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM sessions WHERE room_id = :room_id AND status = :status");
    query.bindValue(":room_id", roomId);
    query.bindValue(":status", SessionModel::STATUS_ACTIVE);

    if (!executeQuery(query, "Count Sessions By Room ID")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int SessionRepository::countByStatus(const QString& status)
{
    if (!checkConnection("Count Sessions By Status")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM sessions WHERE status = :status");
    query.bindValue(":status", status);

    if (!executeQuery(query, "Count Sessions By Status")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int SessionRepository::countAll()
{
    if (!checkConnection("Count All Sessions")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM sessions");

    if (!executeQuery(query, "Count All Sessions")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

bool SessionRepository::exists(const QString& sessionId)
{
    if (!checkConnection("Check Session Exists")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM sessions WHERE session_id = :session_id");
    query.bindValue(":session_id", sessionId);

    if (!executeQuery(query, "Check Session Exists")) {
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

// 私有辅助方法
SessionModel SessionRepository::mapToModel(const QSqlRecord& record)
{
    SessionModel model;
    model.id = record.value("id").toInt();
    model.sessionId = record.value("session_id").toString();
    model.userId = record.value("user_id").toInt();
    model.roomId = record.value("room_id").toString();
    model.status = record.value("status").toString();
    model.createdAt = record.value("created_at").toDateTime();
    model.lastActivity = record.value("last_activity").toDateTime();
    
    if (!record.value("expires_at").isNull()) {
        model.expiresAt = record.value("expires_at").toDateTime();
    }
    
    return model;
}

bool SessionRepository::executeSessionQuery(QSqlQuery& query, const QString& operation)
{
    if (!query.exec()) {
        DBLogger::error("Session Repository", QString("%1 failed: %2").arg(operation).arg(query.lastError().text()));
        return false;
    }
    return true;
}
