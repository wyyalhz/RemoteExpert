#include "user_manager.h"

UserManager::UserManager(QObject *parent) : QObject(parent) {}

UserManager::~UserManager() {}

bool UserManager::validateUser(const QString &username, const QString &password)
{
    if (!db_.isOpen()) {
        qCritical() << "[UserManager] Validate user failed: Database is NOT open!";
        return false;
    }

    QSqlQuery query(db_);
    query.prepare("SELECT password_hash FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec()) {
        qCritical() << "[UserManager] Query user failed:" << query.lastError().text();
        return false;
    }
    
    if (!query.next()) {
        qWarning() << "[UserManager] User not found:" << username;
        return false;
    }

    QString storedHash = query.value(0).toString();
    qDebug() << "[UserManager] Stored hash for" << username << ":" << storedHash;

    QString inputHash = hashPassword(password);
    qDebug() << "[UserManager] Input password hash:" << inputHash;

    bool isPasswordCorrect = (storedHash == inputHash);
    if (isPasswordCorrect) {
        qInfo() << "[UserManager] User validate SUCCESS:" << username;
    } else {
        qWarning() << "[UserManager] User validate FAILED: Wrong password for" << username;
    }
    
    return isPasswordCorrect;
}

bool UserManager::registerUser(const QString &username, const QString &password, const QString &role)
{
    if (!db_.isOpen()) {
        qCritical() << "[UserManager] Register user failed: Database is NOT open!";
        return false;
    }

    // 检查用户是否已存在
    if (userExists(username)) {
        qWarning() << "[UserManager] User already exists:" << username;
        return false;
    }

    QSqlQuery query(db_);
    query.prepare("INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(hashPassword(password));
    query.addBindValue(role);

    if (!query.exec()) {
        qCritical() << "[UserManager] Register user failed:" << query.lastError().text();
        return false;
    }

    qInfo() << "[UserManager] User registered successfully:" << username;
    return true;
}

int UserManager::getUserId(const QString &username)
{
    if (!db_.isOpen()) {
        return -1;
    }

    QSqlQuery query(db_);
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec() || !query.next()) {
        return -1;
    }

    return query.value(0).toInt();
}

QString UserManager::getUserRole(const QString &username)
{
    if (!db_.isOpen()) {
        return QString();
    }

    QSqlQuery query(db_);
    query.prepare("SELECT role FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec() || !query.next()) {
        return QString();
    }

    return query.value(0).toString();
}

bool UserManager::userExists(const QString &username)
{
    if (!db_.isOpen()) {
        return false;
    }

    QSqlQuery query(db_);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

QString UserManager::hashPassword(const QString &password)
{
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hashBytes = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    return hashBytes.toHex();
}
