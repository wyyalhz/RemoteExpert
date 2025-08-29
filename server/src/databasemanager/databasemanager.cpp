#include "databasemanager.h"
#include "user/user_manager.h"
#include "workorder/workorder_manager.h"
#include "logger/db_logger.h"

DatabaseManager::DatabaseManager(QObject *parent) 
    : QObject(parent)
    , userManager_(nullptr)
    , workOrderManager_(nullptr)
    , logger_(nullptr)
{
}

DatabaseManager::~DatabaseManager()
{
    if (db_.isOpen()) {
        db_.close();
    }
    
    delete userManager_;
    delete workOrderManager_;
    delete logger_;
}

bool DatabaseManager::initialize()
{
    // 初始化数据库连接
    QString connectionName = "remote_support_connection";
    if (QSqlDatabase::contains(connectionName)) {
        db_ = QSqlDatabase::database(connectionName);
    } else {
        db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }

    // 确保数据库目录存在
    QString dbDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dbDir(dbDirPath);
    if (!dbDir.exists() && !dbDir.mkpath(".")) {
        qCritical() << "Failed to create database directory:" << dbDirPath;
        return false;
    }

    QString dbPath = dbDir.filePath("remote_support.db");
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

    // 初始化子管理器
    userManager_ = new UserManager(this);
    workOrderManager_ = new WorkOrderManager(this);
    logger_ = new DBLogger(this);
    
    // 设置数据库连接
    userManager_->setDatabase(db_);
    workOrderManager_->setDatabase(db_);
    logger_->setDatabase(db_);

    qInfo() << "Database initialized successfully! All managers are ready.";
    return true;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(db_);

    // 创建用户表
    QString createUserTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT 'operator',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createUserTable)) {
        qCritical() << "Failed to create users table:" << query.lastError().text();
        return false;
    }

    // 创建工单表
    QString createWorkOrderTable = R"(
        CREATE TABLE IF NOT EXISTS work_orders(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ticket_id TEXT UNIQUE NOT NULL,
            title TEXT NOT NULL,                           -- 工单标题（问题简述）
            description TEXT,                              -- 工单详细描述
            creator_id INTEGER NOT NULL,
            status TEXT DEFAULT 'open',                    -- 工单状态：open/processing/closed
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

    qInfo() << "Database tables created successfully!";
    return true;
}
