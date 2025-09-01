#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include "Business/services/AuthService.h"
#include <QMouseEvent>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    QString getCurrentUser() const { return currentUser; }
    int getCurrentUserType() const { return currentUserType; }

    // 设置认证服务
    void setAuthService(AuthService* authService);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onUserTypeChanged(int index);

    void on_titleLabel_linkActivated(const QString &link);

    void on_usernameEdit_textChanged(const QString &arg1);

    void on_userTypeCombo_currentIndexChanged(int index);

private:
    Ui::LoginDialog *ui;
    QString currentUser;
    int currentUserType;
    //ui
    QPoint m_dragStart;
    bool   m_dragging = false;
    
    // 认证服务引用
    AuthService* authService_;

//ui
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

};

#endif // LOGINDIALOG_H
