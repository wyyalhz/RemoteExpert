#include "user_manager.h"

UserManager::UserManager(QObject *parent) : DBBase(parent) {}

UserManager::~UserManager() {}

bool UserManager::validateUser(const QString &username, const QString &password)
{
    if (!checkConnection("Validate user")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT password_hash FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeQuery(query, "Query user")) {
        return false;
    }
    
    if (!checkQueryResult(query, "User not found")) {
        logWarning("Validate user", QString("User not found: %1").arg(username));
        return false;
    }

    QString storedHash = query.value(0).toString();
    qDebug() << QString("[%1] Stored hash for %2: %3").arg(metaObject()->className()).arg(username).arg(storedHash);

    QString inputHash = hashPassword(password);
    qDebug() << QString("[%1] Input password hash: %2").arg(metaObject()->className()).arg(inputHash);

    bool isPasswordCorrect = (storedHash == inputHash);
    if (isPasswordCorrect) {
        logInfo("Validate user", QString("User validate SUCCESS: %1").arg(username));
    } else {
        logWarning("Validate user", QString("User validate FAILED: Wrong password for %1").arg(username));
    }
    
    return isPasswordCorrect;
}

bool UserManager::registerUser(const QString &username, const QString &password, const QString &role)
{
    if (!checkConnection("Register user")) {
        return false;
    }

    // 检查用户是否已存在
    if (userExists(username)) {
        logWarning("Register user", QString("User already exists: %1").arg(username));
        return false;
    }

    QSqlQuery query(database());
    query.prepare("INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(hashPassword(password));
    query.addBindValue(role);

    if (!executeQuery(query, "Register user")) {
        return false;
    }

    logInfo("Register user", QString("User registered successfully: %1").arg(username));
    return true;
}

int UserManager::getUserId(const QString &username)
{
    if (!checkConnection("Get user ID")) {
        return -1;
    }

    QSqlQuery query(database());
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeQuery(query, "Get user ID") || !checkQueryResult(query, "User not found")) {
        return -1;
    }

    return query.value(0).toInt();
}

QString UserManager::getUserRole(const QString &username)
{
    if (!checkConnection("Get user role")) {
        return QString();
    }

    QSqlQuery query(database());
    query.prepare("SELECT role FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeQuery(query, "Get user role") || !checkQueryResult(query, "User not found")) {
        return QString();
    }

    return query.value(0).toString();
}

bool UserManager::userExists(const QString &username)
{
    if (!checkConnection("Check user exists")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeQuery(query, "Check user exists") || !checkQueryResult(query, "Count query failed")) {
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
