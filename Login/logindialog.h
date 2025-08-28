#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include "databasemanager.h"

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

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onUserTypeChanged(int index);

private:
    Ui::LoginDialog *ui;
    QString currentUser;
    int currentUserType;
};

#endif // LOGINDIALOG_H
