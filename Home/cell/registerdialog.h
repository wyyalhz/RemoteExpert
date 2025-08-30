#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

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

private slots:
    void onRegisterClicked();
    void onCancelClicked();
    void validateInput();

private:
    Ui::RegisterDialog *ui;
    QString m_username;
    QString m_email;
    QString m_phone;
    int m_userType;
};

#endif // REGISTERDIALOG_H
