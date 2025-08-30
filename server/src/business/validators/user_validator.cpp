#include "user_validator.h"
#include "../logging/business_logger.h"
#include <QRegularExpression>

void UserValidator::validateRegistration(const QString& username, const QString& password, 
                                       const QString& email, const QString& phone, int userType)
{
    BusinessLogger::businessOperationStart("User Registration Validation");
    
    try {
        validateUsername(username);
        validatePassword(password);
        validateUserType(userType);
        
        if (!email.isEmpty()) {
            validateEmail(email);
        }
        
        if (!phone.isEmpty()) {
            validatePhone(phone);
        }
        
        BusinessLogger::validationSuccess("User Registration", "All fields validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("User Registration", e.getField(), e.getMessage());
        throw;
    }
}

void UserValidator::validateLogin(const QString& username, const QString& password, int userType)
{
    BusinessLogger::businessOperationStart("User Login Validation");
    
    try {
        validateUsername(username);
        validatePassword(password);
        validateUserType(userType);
        
        BusinessLogger::validationSuccess("User Login", "All fields validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("User Login", e.getField(), e.getMessage());
        throw;
    }
}

void UserValidator::validateUserModel(const UserModel& user)
{
    BusinessLogger::businessOperationStart("User Model Validation");
    
    try {
        if (!user.isValid()) {
            throw ValidationException("User model is invalid", "model", "User Model Validation");
        }
        
        validateUsername(user.username);
        validateUserType(user.userType);
        
        if (!user.email.isEmpty()) {
            validateEmail(user.email);
        }
        
        if (!user.phone.isEmpty()) {
            validatePhone(user.phone);
        }
        
        BusinessLogger::validationSuccess("User Model", "User model validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("User Model", e.getField(), e.getMessage());
        throw;
    }
}

void UserValidator::validateUserUpdate(const UserModel& user)
{
    BusinessLogger::businessOperationStart("User Update Validation");
    
    try {
        if (user.id <= 0) {
            throw ValidationException("User ID must be positive", "id", "User Update Validation");
        }
        
        validateUserModel(user);
        
        BusinessLogger::validationSuccess("User Update", "User update data validated successfully");
    }
    catch (const ValidationException& e) {
        BusinessLogger::validationFailed("User Update", e.getField(), e.getMessage());
        throw;
    }
}

void UserValidator::validatePassword(const QString& password)
{
    if (password.isEmpty()) {
        throw ValidationException("Password cannot be empty", "password", "Password Validation");
    }
    
    if (password.length() < 6) {
        throw ValidationException("Password must be at least 6 characters long", "password", "Password Validation");
    }
    
    if (!isStrongPassword(password)) {
        throw ValidationException("Password is too weak", "password", "Password Validation");
    }
}

void UserValidator::validateUsername(const QString& username)
{
    if (username.isEmpty()) {
        throw ValidationException("Username cannot be empty", "username", "Username Validation");
    }
    
    if (username.length() < 3) {
        throw ValidationException("Username must be at least 3 characters long", "username", "Username Validation");
    }
    
    if (username.length() > 50) {
        throw ValidationException("Username cannot exceed 50 characters", "username", "Username Validation");
    }
    
    if (!isValidUsernameFormat(username)) {
        throw ValidationException("Username contains invalid characters", "username", "Username Validation");
    }
}

void UserValidator::validateEmail(const QString& email)
{
    if (email.isEmpty()) {
        throw ValidationException("Email cannot be empty", "email", "Email Validation");
    }
    
    if (!isValidEmailFormat(email)) {
        throw ValidationException("Invalid email format", "email", "Email Validation");
    }
}

void UserValidator::validatePhone(const QString& phone)
{
    if (phone.isEmpty()) {
        throw ValidationException("Phone number cannot be empty", "phone", "Phone Validation");
    }
    
    if (!isValidPhoneFormat(phone)) {
        throw ValidationException("Invalid phone number format", "phone", "Phone Validation");
    }
}

void UserValidator::validateUserType(int userType)
{
    if (userType != FACTORY_USER && userType != EXPERT_USER) {
        throw ValidationException("Invalid user type", "userType", "User Type Validation");
    }
}

// 私有辅助方法
bool UserValidator::isValidEmailFormat(const QString& email)
{
    QRegularExpression emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return emailRegex.match(email).hasMatch();
}

bool UserValidator::isValidPhoneFormat(const QString& phone)
{
    QRegularExpression phoneRegex(R"(^1[3-9]\d{9}$)");
    return phoneRegex.match(phone).hasMatch();
}

bool UserValidator::isValidUsernameFormat(const QString& username)
{
    QRegularExpression usernameRegex(R"(^[a-zA-Z0-9_-]+$)");
    return usernameRegex.match(username).hasMatch();
}

bool UserValidator::isStrongPassword(const QString& password)
{
    // 检查密码强度：至少包含字母和数字
    bool hasLetter = false;
    bool hasDigit = false;
    
    for (const QChar& ch : password) {
        if (ch.isLetter()) {
            hasLetter = true;
        } else if (ch.isDigit()) {
            hasDigit = true;
        }
    }
    
    return hasLetter && hasDigit;
}
