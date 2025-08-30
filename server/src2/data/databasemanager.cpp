#include "databasemanager.h"
#include "repositories/workorder_repository.h"
#include "repositories/user_repository.h"
#include "repositories/session_repository.h"

DatabaseManager::DatabaseManager(QObject *parent) 
    : QObject(parent)
    , workOrderRepo_(nullptr)
    , userRepo_(nullptr)
    , sessionRepo_(nullptr)
{
}

DatabaseManager::~DatabaseManager()
{
    if (db_.isOpen()) {
        db_.close();
    }
    
    delete workOrderRepo_;
    delete userRepo_;
    delete sessionRepo_;
}

bool DatabaseManager::initialize()
{
    // 确保数据库目录存在
    if (!ensureDatabaseDirectory()) {
        return false;
    }

    // 初始化数据库连接
    QString connectionName = "remote_support_connection";
    if (QSqlDatabase::contains(connectionName)) {
        db_ = QSqlDatabase::database(connectionName);
    } else {
        db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }

    QString dbPath = QDir::currentPath() + "/database/remote_support.db";
    qInfo() << "Database path:" << dbPath;

    db_.setDatabaseName(dbPath);

    if (!db_.open()) {
        qCritical() << "Failed to open database:" << db_.lastError().text();
        return false;
    }

    // 创建数据库表
    if (!createTables()) {
        return false;
    }

    // 初始化Repository实例
    workOrderRepo_ = new WorkOrderRepository(this);
    userRepo_ = new UserRepository(this);
    sessionRepo_ = new SessionRepository(this);
    
    // 设置数据库连接
    workOrderRepo_->setDatabase(db_);
    userRepo_->setDatabase(db_);
    sessionRepo_->setDatabase(db_);

    qInfo() << "Database initialized successfully! All repositories are ready.";
    return true;
}

bool DatabaseManager::ensureDatabaseDirectory()
{
    QString dbDirPath = QDir::currentPath() + "/database";
    QDir dbDir(dbDirPath);
    if (!dbDir.exists() && !dbDir.mkpath(".")) {
        qCritical() << QString("Failed to create database directory: %1").arg(dbDirPath);
        return false;
    }
    return true;
}

bool DatabaseManager::createTables()
{
    return createWorkOrderTables() && 
           createUserTables() && 
           createSessionTables();
}

bool DatabaseManager::createWorkOrderTables()
{
    QSqlQuery query(db_);

    // 创建工单表
    QString createWorkOrderTable = R"(
        CREATE TABLE IF NOT EXISTS work_orders(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ticket_id TEXT UNIQUE NOT NULL,
            title TEXT NOT NULL,                           -- 工单标题（问题简述）
            description TEXT,                              -- 工单详细描述
            creator_id INTEGER NOT NULL,
            status TEXT DEFAULT 'open',                    -- 工单状态：open/processing/refused/closed
            priority TEXT DEFAULT 'normal',                -- 优先级：low/normal/high/urgent
            category TEXT,                                 -- 工单分类（如：设备故障、软件问题等）
            assigned_to INTEGER,                           -- 指派给专家ID
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP, -- 最后更新时间
            closed_at DATETIME,
            FOREIGN KEY (creator_id) REFERENCES users (id),
            FOREIGN KEY (assigned_to) REFERENCES users (id)
        )
    )";
    
    if (!query.exec(createWorkOrderTable)) {
        qCritical() << "Failed to create work_orders table:" << query.lastError().text();
        return false;
    }

    // 创建工单参与者表
    QString createWorkOrderParticipantsTable = R"(
        CREATE TABLE IF NOT EXISTS work_order_participants(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            work_order_id INTEGER NOT NULL,                -- 工单ID
            user_id INTEGER NOT NULL,                      -- 用户ID
            role TEXT NOT NULL,                           -- 角色：creator/expert/operator/viewer
            joined_at DATETIME DEFAULT CURRENT_TIMESTAMP, -- 加入时间
            left_at DATETIME,                             -- 离开时间（支持重新加入）
            permissions TEXT,                             -- 权限设置（如：can_chat, can_video等）
            FOREIGN KEY (work_order_id) REFERENCES work_orders (id),
            FOREIGN KEY (user_id) REFERENCES users (id),
            UNIQUE(work_order_id, user_id)                -- 防止重复加入同一工单
        )
    )";
    
    if (!query.exec(createWorkOrderParticipantsTable)) {
        qCritical() << "Failed to create work_order_participants table:" << query.lastError().text();
        return false;
    }

    qInfo() << "Work order tables created successfully!";
    return true;
}

bool DatabaseManager::createUserTables()
{
    QSqlQuery query(db_);

    // 创建用户表
    QString createUserTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            email TEXT,
            phone TEXT,
            user_type INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createUserTable)) {
        qCritical() << "Failed to create users table:" << query.lastError().text();
        return false;
    }

    qInfo() << "User tables created successfully!";
    return true;
}

bool DatabaseManager::createSessionTables()
{
    QSqlQuery query(db_);

    // 创建会话表（预留实现位置）
    QString createSessionTable = R"(
        CREATE TABLE IF NOT EXISTS sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            session_id TEXT UNIQUE NOT NULL,
            user_id INTEGER NOT NULL,
            room_id TEXT,                                  -- 房间ID（工单ID）
            status TEXT DEFAULT 'active',                 -- 会话状态：active/inactive/expired
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_activity DATETIME DEFAULT CURRENT_TIMESTAMP,
            expires_at DATETIME,                          -- 会话过期时间
            FOREIGN KEY (user_id) REFERENCES users (id)
        )
    )";

    if (!query.exec(createSessionTable)) {
        qCritical() << "Failed to create sessions table:" << query.lastError().text();
        return false;
    }

    qInfo() << "Session tables created successfully!";
    return true;
}

WorkOrderRepository* DatabaseManager::workOrderRepository() const
{
    return workOrderRepo_;
}

UserRepository* DatabaseManager::userRepository() const
{
    return userRepo_;
}

SessionRepository* DatabaseManager::sessionRepository() const
{
    return sessionRepo_;
}

bool DatabaseManager::beginTransaction()
{
    return db_.transaction();
}

bool DatabaseManager::commitTransaction()
{
    return db_.commit();
}

bool DatabaseManager::rollbackTransaction()
{
    return db_.rollback();
}

QSqlDatabase& DatabaseManager::database()
{
    return db_;
}

bool DatabaseManager::isConnected() const
{
    return db_.isOpen();
}


