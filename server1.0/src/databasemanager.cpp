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
                                   creator_id INTEGER NOT NULL,
                                   status TEXT DEFAULT 'open',
                                   created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                                   closed_at DATETIME,
                                   FOREIGN KEY (creator_id) REFERENCES users (id)
                                   )
                                   )";
    if(!query.exec(createWorkOrderTable))
    {
        qCritical()<<"Failed to create work_orders table:"<<query.lastError().text();
        return false;
    }
    qInfo() << "Database initialized successfully! Tables (users/work_orders) are ready.";
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
