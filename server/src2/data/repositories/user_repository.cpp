#include "user_repository.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QCryptographicHash>

UserRepository::UserRepository(QObject *parent) : DBBase(parent) {}

UserRepository::~UserRepository() {}

// 基础CRUD操作
bool UserRepository::create(const UserModel& user, int& userId)
{
    if (!checkConnection("Create user")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("INSERT INTO users (username, password_hash, email, phone, user_type) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(user.username);
    query.addBindValue(user.passwordHash);
    query.addBindValue(user.email);
    query.addBindValue(user.phone);
    query.addBindValue(user.userType);

    if (!executeUserQuery(query, "Create user")) {
        return false;
    }

    userId = query.lastInsertId().toInt();
    logInfo("Create user", QString("User created successfully: %1 ID: %2").arg(user.username).arg(userId));
    return true;
}

bool UserRepository::findById(int userId, UserModel& user)
{
    if (!checkConnection("Find user by ID")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM users WHERE id = ?");
    query.addBindValue(userId);

    if (!executeUserQuery(query, "Find user by ID")) {
        return false;
    }

    if (query.next()) {
        user = mapToModel(query.record());
        return true;
    }

    return false;
}

bool UserRepository::findByUsername(const QString& username, UserModel& user)
{
    if (!checkConnection("Find user by username")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeUserQuery(query, "Find user by username")) {
        return false;
    }

    if (query.next()) {
        user = mapToModel(query.record());
        return true;
    }

    return false;
}

bool UserRepository::update(const UserModel& user)
{
    if (!checkConnection("Update user")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare(R"(
        UPDATE users 
        SET username = ?, password_hash = ?, email = ?, phone = ?, user_type = ? 
        WHERE id = ?
    )");
    
    query.addBindValue(user.username);
    query.addBindValue(user.passwordHash);
    query.addBindValue(user.email);
    query.addBindValue(user.phone);
    query.addBindValue(user.userType);
    query.addBindValue(user.id);

    return executeUserQuery(query, "Update user");
}

bool UserRepository::remove(int userId)
{
    if (!checkConnection("Remove user")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("DELETE FROM users WHERE id = ?");
    query.addBindValue(userId);

    return executeUserQuery(query, "Remove user");
}

// 用户管理
bool UserRepository::validateUser(const QString& username, const QString& password, int userType)
{
    if (!checkConnection("Validate user")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT password_hash FROM users WHERE username = ? AND user_type = ?");
    query.addBindValue(username);
    query.addBindValue(userType);

    if (!executeUserQuery(query, "Query user")) {
        return false;
    }
    
    if (!checkQueryResult(query, "User not found")) {
        logWarning("Validate user", QString("User not found: %1 (user_type: %2)").arg(username).arg(userType));
        return false;
    }

    QString storedHash = query.value(0).toString();
    QString inputHash = hashPassword(password);

    bool isPasswordCorrect = (storedHash == inputHash);
    if (isPasswordCorrect) {
        logInfo("Validate user", QString("User validate SUCCESS: %1 (user_type: %2)").arg(username).arg(userType));
    } else {
        logWarning("Validate user", QString("User validate FAILED: Wrong password for %1").arg(username));
    }
    
    return isPasswordCorrect;
}

bool UserRepository::registerUser(const QString& username, const QString& password, 
                                 const QString& email, const QString& phone, int userType)
{
    if (!checkConnection("Register user")) {
        return false;
    }

    // 检查用户是否已存在
    if (exists(username)) {
        logWarning("Register user", QString("User already exists: %1").arg(username));
        return false;
    }

    QSqlQuery query(database());
    query.prepare("INSERT INTO users (username, password_hash, email, phone, user_type) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(hashPassword(password));
    query.addBindValue(email);
    query.addBindValue(phone);
    query.addBindValue(userType);

    if (!executeUserQuery(query, "Register user")) {
        return false;
    }

    logInfo("Register user", QString("User registered successfully: %1 (user_type: %2)").arg(username).arg(userType));
    return true;
}

bool UserRepository::updatePassword(int userId, const QString& newPasswordHash)
{
    if (!checkConnection("Update password")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("UPDATE users SET password_hash = ? WHERE id = ?");
    query.addBindValue(newPasswordHash);
    query.addBindValue(userId);

    return executeUserQuery(query, "Update password");
}

bool UserRepository::updateUserType(int userId, int userType)
{
    if (!checkConnection("Update user type")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("UPDATE users SET user_type = ? WHERE id = ?");
    query.addBindValue(userType);
    query.addBindValue(userId);

    return executeUserQuery(query, "Update user type");
}

// 查询操作
QList<UserModel> UserRepository::findByUserType(int userType)
{
    QList<UserModel> users;
    
    if (!checkConnection("Find users by type")) {
        return users;
    }

    QSqlQuery query(database());
    query.prepare("SELECT * FROM users WHERE user_type = ? ORDER BY created_at DESC");
    query.addBindValue(userType);

    if (!executeUserQuery(query, "Find users by type")) {
        return users;
    }

    while (query.next()) {
        users.append(mapToModel(query.record()));
    }

    return users;
}

QList<UserModel> UserRepository::findAll(int limit, int offset)
{
    QList<UserModel> users;
    
    if (!checkConnection("Find all users")) {
        return users;
    }

    QString sql = "SELECT * FROM users ORDER BY created_at DESC";
    if (limit > 0) {
        sql += QString(" LIMIT %1 OFFSET %2").arg(limit).arg(offset);
    }

    QSqlQuery query(database());
    query.prepare(sql);

    if (!executeUserQuery(query, "Find all users")) {
        return users;
    }

    while (query.next()) {
        users.append(mapToModel(query.record()));
    }

    return users;
}

// 统计查询
int UserRepository::countByUserType(int userType)
{
    if (!checkConnection("Count users by type")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM users WHERE user_type = ?");
    query.addBindValue(userType);

    if (!executeUserQuery(query, "Count users by type")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int UserRepository::countAll()
{
    if (!checkConnection("Count all users")) {
        return 0;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM users");

    if (!executeUserQuery(query, "Count all users")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

bool UserRepository::exists(const QString& username)
{
    if (!checkConnection("Check user exists")) {
        return false;
    }

    QSqlQuery query(database());
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeUserQuery(query, "Check user exists")) {
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

// 获取用户信息
int UserRepository::getUserId(const QString& username)
{
    if (!checkConnection("Get user ID")) {
        return -1;
    }

    QSqlQuery query(database());
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeUserQuery(query, "Get user ID") || !checkQueryResult(query, "User not found")) {
        return -1;
    }

    return query.value(0).toInt();
}

int UserRepository::getUserType(const QString& username)
{
    if (!checkConnection("Get user type")) {
        return -1;
    }

    QSqlQuery query(database());
    query.prepare("SELECT user_type FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeUserQuery(query, "Get user type") || !checkQueryResult(query, "User not found")) {
        return -1;
    }

    return query.value(0).toInt();
}

QString UserRepository::getUserEmail(const QString& username)
{
    if (!checkConnection("Get user email")) {
        return QString();
    }

    QSqlQuery query(database());
    query.prepare("SELECT email FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeUserQuery(query, "Get user email") || !checkQueryResult(query, "User not found")) {
        return QString();
    }

    return query.value(0).toString();
}

QString UserRepository::getUserPhone(const QString& username)
{
    if (!checkConnection("Get user phone")) {
        return QString();
    }

    QSqlQuery query(database());
    query.prepare("SELECT phone FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!executeUserQuery(query, "Get user phone") || !checkQueryResult(query, "User not found")) {
        return QString();
    }

    return query.value(0).toString();
}

// 私有辅助方法
UserModel UserRepository::mapToModel(const QSqlRecord& record)
{
    UserModel model;
    model.id = record.value("id").toInt();
    model.username = record.value("username").toString();
    model.passwordHash = record.value("password_hash").toString();
    model.email = record.value("email").toString();
    model.phone = record.value("phone").toString();
    model.userType = record.value("user_type").toInt();
    model.createdAt = record.value("created_at").toDateTime();
    return model;
}

bool UserRepository::executeUserQuery(QSqlQuery& query, const QString& operation)
{
    return executeQuery(query, operation);
}

QString UserRepository::hashPassword(const QString& password)
{
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hashBytes = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    return hashBytes.toHex();
}
