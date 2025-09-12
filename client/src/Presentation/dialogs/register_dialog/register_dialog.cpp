#include "register_dialog.h"
#include "ui_register_dialog.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog),
    m_userType(0),
    authService_(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("用户注册");

    // 设置验证器
    QRegularExpression phoneRegex("^1[3-9]\\d{9}$");
    ui->phoneEdit->setValidator(new QRegularExpressionValidator(phoneRegex, this));

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    ui->emailEdit->setValidator(new QRegularExpressionValidator(emailRegex, this));

    // 连接信号槽
    connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &RegisterDialog::onCancelClicked);

    // 实时验证输入
    connect(ui->usernameEdit, &QLineEdit::textChanged, this, &RegisterDialog::validateInput);
    connect(ui->passwordEdit, &QLineEdit::textChanged, this, &RegisterDialog::validateInput);
    connect(ui->confirmEdit, &QLineEdit::textChanged, this, &RegisterDialog::validateInput);
    connect(ui->emailEdit, &QLineEdit::textChanged, this, &RegisterDialog::validateInput);
    connect(ui->phoneEdit, &QLineEdit::textChanged, this, &RegisterDialog::validateInput);

    // 设置用户类型选项
    ui->userTypeCombo->addItem("工厂用户");
    ui->userTypeCombo->addItem("技术专家");
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::setAuthService(AuthService* authService)
{
    authService_ = authService;
    
    if (authService_) {
        // 连接认证服务的信号
        connect(authService_, &AuthService::registerSuccess, 
                this, &RegisterDialog::onRegisterSuccess);
        connect(authService_, &AuthService::registerFailed, 
                this, &RegisterDialog::onRegisterFailed);
    }
}

void RegisterDialog::onRegisterClicked()
{
    if (!authService_) {
        QMessageBox::critical(this, "错误", "认证服务未初始化");
        return;
    }

    m_username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString confirm = ui->confirmEdit->text();
    m_email = ui->emailEdit->text().trimmed();
    m_phone = ui->phoneEdit->text().trimmed();
    m_userType = ui->userTypeCombo->currentIndex();

    // 输入验证
    if(m_username.isEmpty() || password.isEmpty()){
        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
        return;
    }

    if(m_username.length() < 3){
        QMessageBox::warning(this, "错误", "用户名长度至少3个字符");
        return;
    }

    if(password.length() < 6){
        QMessageBox::warning(this, "错误", "密码长度至少6个字符");
        return;
    }

    if(password != confirm){
        QMessageBox::warning(this, "错误", "两次输入的密码不一致");
        return;
    }

    if(!m_email.isEmpty() && !ui->emailEdit->hasAcceptableInput()){
        QMessageBox::warning(this, "错误", "邮箱格式不正确");
        return;
    }

    if(!m_phone.isEmpty() && !ui->phoneEdit->hasAcceptableInput()){
        QMessageBox::warning(this, "错误", "手机号格式不正确");
        return;
    }

    // 显示加载状态
    showLoading(true);
    
    // 通过认证服务发送注册请求
    bool success = authService_->registerUser(m_username, password, m_email, m_phone, m_userType);
    
    if (!success) {
        showLoading(false);
        QMessageBox::warning(this, "错误", authService_->getLastError());
    }
}

void RegisterDialog::onRegisterSuccess(const User& user)
{
    showLoading(false);
    
    // 设置用户信息
    if (user.getUsername().isEmpty()) {
        // 如果服务器返回的用户对象没有用户名，使用本地输入的用户名
        User localUser;
        localUser.setUsername(m_username);
        localUser.setEmail(m_email);
        localUser.setPhone(m_phone);
        localUser.setUserType(m_userType);
        QMessageBox::information(this, "成功", "注册成功！");
        accept();
    } else {
        QMessageBox::information(this, "成功", "注册成功！");
        accept();
    }
}

void RegisterDialog::onRegisterFailed(const QString& error)
{
    showLoading(false);
    QMessageBox::warning(this, "注册失败", error);
}

void RegisterDialog::onCancelClicked()
{
    reject();
}

void RegisterDialog::validateInput()
{
    bool valid = true;

    // 检查必填字段
    if(ui->usernameEdit->text().trimmed().length() < 3) valid = false;
    if(ui->passwordEdit->text().length() < 6) valid = false;
    if(ui->passwordEdit->text() != ui->confirmEdit->text()) valid = false;

    // 检查邮箱格式（如果填写了）
    if(!ui->emailEdit->text().isEmpty() && !ui->emailEdit->hasAcceptableInput()) valid = false;

    // 检查手机号格式（如果填写了）
    if(!ui->phoneEdit->text().isEmpty() && !ui->phoneEdit->hasAcceptableInput()) valid = false;

    ui->registerButton->setEnabled(valid);
}

void RegisterDialog::showLoading(bool loading)
{
    ui->registerButton->setEnabled(!loading);
    ui->cancelButton->setEnabled(!loading);
    ui->usernameEdit->setEnabled(!loading);
    ui->passwordEdit->setEnabled(!loading);
    ui->confirmEdit->setEnabled(!loading);
    ui->emailEdit->setEnabled(!loading);
    ui->phoneEdit->setEnabled(!loading);
    ui->userTypeCombo->setEnabled(!loading);
    
    if (loading) {
        ui->registerButton->setText("注册中...");
    } else {
        ui->registerButton->setText("注册");
    }
}
