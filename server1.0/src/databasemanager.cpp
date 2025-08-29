#include "databasemanager.h"

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

DatabaseManager::~DatabaseManager()
{
    if(db_.isOpen())
    {
        db_.close();
    }
}

bool DatabaseManager::initialize()
{
        QString connectionName = "remote_support_connection";
        if (QSqlDatabase::contains(connectionName))
        {
            db_ = QSqlDatabase::database(connectionName);
        }
        else
        {
            db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        }

        // 确保数据库目录存在
        QString dbDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dbDir(dbDirPath);
        if (!dbDir.exists() && !dbDir.mkpath("."))
        {
            qCritical() << "Failed to create database directory:" << dbDirPath;
            return false;
        }

        QString dbPath = dbDir.filePath("remote_support.db");
        qInfo() << "Database path:" << dbPath;

        db_.setDatabaseName(dbPath);

        if (!db_.open())
        {
            qCritical() << "Failed to open database:" << db_.lastError().text();
            return false;
        }

        QSqlQuery query(db_);

    QString createUserTable = R"(
                              CREATE TABLE IF NOT EXISTS users (
                              id INTEGER PRIMARY KEY AUTOINCREMENT,
                              username TEXT UNIQUE NOT NULL,
                              password_hash TEXT NOT NULL,
                              role TEXT NOT NULL DEFAULT 'operator',
                              created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                              )
                              )";

    if(!query.exec(createUserTable))
    {
        qCritical()<<"Failed to create users table:"<<query.lastError().text();
        return false;
    }

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
    if(!query.exec(createWorkOrderTable))
    {
        qCritical()<<"Failed to create work_orders table:"<<query.lastError().text();
        return false;
    }

    // 创建工单参与者表 - 支持多人协作和实时通讯
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
    if(!query.exec(createWorkOrderParticipantsTable))
    {
        qCritical()<<"Failed to create work_order_participants table:"<<query.lastError().text();
        return false;
    }
    qInfo() << "Database initialized successfully! Tables (users/work_orders/work_order_participants) are ready.";
    return true;
}


bool DatabaseManager::validateUser(const QString &username, const QString &password)
{
    if(!db_.isOpen())
    {
        qCritical() << "[DB] Validate user failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    query.prepare("SELECT password_hash FROM users WHERE username = ?");
    query.addBindValue(username);

    if(!query.exec())
    {
        qCritical() << "[DB] Query user failed:" << query.lastError().text();
        return false;
    }
    if(!query.next())
    {
        qWarning() << "[DB] User not found: " << username;
        return false;
    }

    QString storeHash =query.value(0).toString();
    qDebug()<< "[DB] Stored hash for" <<username<<":"<<storeHash;

    QByteArray inputPasswordBytes = password.toUtf8();
    QByteArray inputHashBytes = QCryptographicHash::hash(inputPasswordBytes,QCryptographicHash::Sha256);
    QString inputHash = inputHashBytes.toHex();
    qDebug()<<"[DB] Input password hash:"<<inputHash;

    bool isPasswordCorrect = (storeHash == inputHash);
    if(isPasswordCorrect)
    {
        qInfo()<<"[DB] User validate SUCCESS: "<<username;
    }
    else
    {
        qWarning()<<"[DB] User validate FAILED: Wrong password for"<<username;
    }
    return isPasswordCorrect;
}

// ==================== 工单管理方法实现 ====================

bool DatabaseManager::createWorkOrder(const QString &title, const QString &description,
                                     int creatorId, const QString &priority, const QString &category, QString &generatedTicketId)
{
    if (!db_.isOpen()) {
        qCritical() << "[DB] Create work order failed: Database is NOT open!";
        return false;
    }

    // 自动生成工单号
    QSqlQuery query(db_);
    query.prepare("SELECT MAX(CAST(ticket_id AS INTEGER)) FROM work_orders");
    
    if (!query.exec()) {
        qCritical() << "[DB] Query max work order ID failed:" << query.lastError().text();
        return false;
    }
    
    int maxId = 0;
    if (query.next()) {
        maxId = query.value(0).toInt();
    }
    
    // 生成新的工单号（最大号+1）
    generatedTicketId = QString::number(maxId + 1);
    
    qInfo() << "[DB] Generated work order ID:" << generatedTicketId;

    // 插入工单记录
    query.prepare(R"(
        INSERT INTO work_orders (ticket_id, title, description, creator_id, priority, category, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )");
    
    query.addBindValue(generatedTicketId);
    query.addBindValue(title);
    query.addBindValue(description);
    query.addBindValue(creatorId);
    query.addBindValue(priority);
    query.addBindValue(category);

    if (!query.exec()) {
        qCritical() << "[DB] Create work order failed:" << query.lastError().text();
        return false;
    }

    // 获取新创建的工单ID
    int workOrderId = query.lastInsertId().toInt();
    
    // 自动将创建者添加为参与者
    if (workOrderId > 0) {
        joinWorkOrder(workOrderId, creatorId, "creator");
    }

    qInfo() << "[DB] Work order created successfully:" << generatedTicketId << "ID:" << workOrderId;
    return true;
}

bool DatabaseManager::joinWorkOrder(int workOrderId, int userId, const QString &role)
{
    if (!db_.isOpen()) {
        qCritical() << "[DB] Join work order failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    
    // 检查是否已经参与该工单
    query.prepare("SELECT id FROM work_order_participants WHERE work_order_id = ? AND user_id = ?");
    query.addBindValue(workOrderId);
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qCritical() << "[DB] Check participant failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        // 如果已存在，更新角色和加入时间
        query.prepare(R"(
            UPDATE work_order_participants 
            SET role = ?, joined_at = CURRENT_TIMESTAMP, left_at = NULL 
            WHERE work_order_id = ? AND user_id = ?
        )");
        query.addBindValue(role);
        query.addBindValue(workOrderId);
        query.addBindValue(userId);
    } else {
        // 如果不存在，插入新记录
        query.prepare(R"(
            INSERT INTO work_order_participants (work_order_id, user_id, role, joined_at)
            VALUES (?, ?, ?, CURRENT_TIMESTAMP)
        )");
        query.addBindValue(workOrderId);
        query.addBindValue(userId);
        query.addBindValue(role);
    }

    if (!query.exec()) {
        qCritical() << "[DB] Join work order failed:" << query.lastError().text();
        return false;
    }

    qInfo() << "[DB] User" << userId << "joined work order" << workOrderId << "as" << role;
    return true;
}

bool DatabaseManager::leaveWorkOrder(int workOrderId, int userId)
{
    if (!db_.isOpen()) {
        qCritical() << "[DB] Leave work order failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        UPDATE work_order_participants 
        SET left_at = CURRENT_TIMESTAMP 
        WHERE work_order_id = ? AND user_id = ?
    )");
    
    query.addBindValue(workOrderId);
    query.addBindValue(userId);

    if (!query.exec()) {
        qCritical() << "[DB] Leave work order failed:" << query.lastError().text();
        return false;
    }

    qInfo() << "[DB] User" << userId << "left work order" << workOrderId;
    return true;
}


bool DatabaseManager::closeWorkOrder(int workOrderId, int userId)
{
    if (!db_.isOpen()) {
        qCritical() << "[DB] Close work order failed: Database is NOT open!";
        return false;
    }

    // 检查用户是否为工单创建者
    QSqlQuery checkQuery(db_);
    checkQuery.prepare("SELECT creator_id FROM work_orders WHERE id = ?");
    checkQuery.addBindValue(workOrderId);
    
    if (!checkQuery.exec() || !checkQuery.next()) {
        qCritical() << "[DB] Work order not found:" << workOrderId;
        return false;
    }
    
    int creatorId = checkQuery.value("creator_id").toInt();
    if (creatorId != userId) {
        qWarning() << "[DB] User" << userId << "is not the creator of work order" << workOrderId;
        return false;
    }

    // 更新工单状态为closed
    QSqlQuery updateQuery(db_);
    updateQuery.prepare(R"(
        UPDATE work_orders 
        SET status = 'closed', closed_at = CURRENT_TIMESTAMP, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    updateQuery.addBindValue(workOrderId);

    if (!updateQuery.exec()) {
        qCritical() << "[DB] Close work order failed:" << updateQuery.lastError().text();
        return false;
    }

    qInfo() << "[DB] Work order" << workOrderId << "status updated to closed by user" << userId;
    return true;
}


QJsonArray DatabaseManager::getWorkOrderParticipants(int workOrderId)
{
    QJsonArray participants;
    
    if (!db_.isOpen()) {
        qCritical() << "[DB] Get participants failed: Database is NOT open!";
        return participants;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        SELECT wp.user_id, wp.role, wp.joined_at, wp.left_at, u.username
        FROM work_order_participants wp
        JOIN users u ON wp.user_id = u.id
        WHERE wp.work_order_id = ? AND wp.left_at IS NULL
        ORDER BY wp.joined_at
    )");
    
    query.addBindValue(workOrderId);

    if (!query.exec()) {
        qCritical() << "[DB] Get participants failed:" << query.lastError().text();
        return participants;
    }

    while (query.next()) {
        QJsonObject participant;
        participant["user_id"] = query.value("user_id").toInt();
        participant["username"] = query.value("username").toString();
        participant["role"] = query.value("role").toString();
        participant["joined_at"] = query.value("joined_at").toString();
        participants.append(participant);
    }

    return participants;
}

bool DatabaseManager::updateWorkOrderStatus(int workOrderId, const QString &status)
{
    if (!db_.isOpen()) {
        qCritical() << "[DB] Update status failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        UPDATE work_orders 
        SET status = ?, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    query.addBindValue(status);
    query.addBindValue(workOrderId);

    if (!query.exec()) {
        qCritical() << "[DB] Update status failed:" << query.lastError().text();
        return false;
    }

    qInfo() << "[DB] Work order" << workOrderId << "status updated to" << status;
    return true;
}

bool DatabaseManager::assignWorkOrder(int workOrderId, int expertId)
{
    if (!db_.isOpen()) {
        qCritical() << "[DB] Assign work order failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        UPDATE work_orders 
        SET assigned_to = ?, updated_at = CURRENT_TIMESTAMP 
        WHERE id = ?
    )");
    
    query.addBindValue(expertId);
    query.addBindValue(workOrderId);

    if (!query.exec()) {
        qCritical() << "[DB] Assign work order failed:" << query.lastError().text();
        return false;
    }

    qInfo() << "[DB] Work order" << workOrderId << "assigned to expert" << expertId;
    return true;
}

QJsonObject DatabaseManager::getWorkOrderInfo(int workOrderId)
{
    QJsonObject workOrder;
    
    if (!db_.isOpen()) {
        qCritical() << "[DB] Get work order info failed: Database is NOT open!";
        return workOrder;
    }

    QSqlQuery query(db_);
    query.prepare(R"(
        SELECT wo.*, u.username as creator_name, e.username as assigned_expert_name
        FROM work_orders wo
        JOIN users u ON wo.creator_id = u.id
        LEFT JOIN users e ON wo.assigned_to = e.id
        WHERE wo.id = ?
    )");
    
    query.addBindValue(workOrderId);

    if (!query.exec()) {
        qCritical() << "[DB] Get work order info failed:" << query.lastError().text();
        return workOrder;
    }

    if (query.next()) {
        workOrder["id"] = query.value("id").toInt();
        workOrder["ticket_id"] = query.value("ticket_id").toString();
        workOrder["title"] = query.value("title").toString();
        workOrder["description"] = query.value("description").toString();
        workOrder["status"] = query.value("status").toString();
        workOrder["priority"] = query.value("priority").toString();
        workOrder["category"] = query.value("category").toString();
        workOrder["creator_id"] = query.value("creator_id").toInt();
        workOrder["creator_name"] = query.value("creator_name").toString();
        workOrder["assigned_to"] = query.value("assigned_to").toInt();
        workOrder["assigned_expert_name"] = query.value("assigned_expert_name").toString();
        workOrder["created_at"] = query.value("created_at").toString();
        workOrder["updated_at"] = query.value("updated_at").toString();
        workOrder["closed_at"] = query.value("closed_at").toString();
    }

    return workOrder;
}
