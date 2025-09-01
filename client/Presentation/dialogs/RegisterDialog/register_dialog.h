#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "../../Business/services/AuthService.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

    QString getUsername() const { return m_username; }
    QString getEmail() const { return m_email; }
    QString getPhone() const { return m_phone; }
    int getUserType() const { return m_userType; }

    // 设置认证服务
    void setAuthService(AuthService* authService);

private slots:
    void onRegisterClicked();
    void onCancelClicked();
    void validateInput();
    
    // 认证服务响应处理
    void onRegisterSuccess(const User& user);
    void onRegisterFailed(const QString& error);

private:
    Ui::RegisterDialog *ui;
    QString m_username;
    QString m_email;
    QString m_phone;
    int m_userType;
    
    // 认证服务引用
    AuthService* authService_;
    
    // 显示加载状态
    void showLoading(bool loading);
};

#endif // REGISTERDIALOG_H
