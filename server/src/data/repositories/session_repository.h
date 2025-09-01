#ifndef SESSION_REPOSITORY_H
#define SESSION_REPOSITORY_H

#include "../base/db_base.h"
#include "../models/session_model.h"
#include <QList>

class SessionRepository : public DBBase
{
    Q_OBJECT
public:
    explicit SessionRepository(QObject *parent = nullptr);
    ~SessionRepository();

    // 基础CRUD操作（预留实现位置）
    bool create(const SessionModel& session, int& sessionId);
    bool findById(int sessionId, SessionModel& session);
    bool findBySessionId(const QString& sessionId, SessionModel& session);
    bool update(const SessionModel& session);
    bool remove(int sessionId);
    
    // 会话管理（预留实现位置）
    bool updateLastActivity(int sessionId);
    bool updateStatus(int sessionId, const QString& status);
    bool expireSession(int sessionId);
    
    // 查询操作（预留实现位置）
    QList<SessionModel> findByUserId(int userId);
    QList<SessionModel> findByRoomId(const QString& roomId);
    QList<SessionModel> findByStatus(const QString& status);
    QList<SessionModel> findExpiredSessions();
    QList<SessionModel> findAll(int limit = -1, int offset = 0);
    
    // 统计查询（预留实现位置）
    int countByUserId(int userId);
    int countByRoomId(const QString& roomId);
    int countByStatus(const QString& status);
    int countAll();
    bool exists(const QString& sessionId);

private:
    // 私有辅助方法（预留实现位置）
    SessionModel mapToModel(const QSqlRecord& record);
    bool executeSessionQuery(QSqlQuery& query, const QString& operation);
};

#endif // SESSION_REPOSITORY_H
