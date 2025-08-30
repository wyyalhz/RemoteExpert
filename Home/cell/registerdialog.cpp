#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog),
    m_userType(0)
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

void RegisterDialog::onRegisterClicked()
{
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

    // 检查用户是否存在
    if(DatabaseManager::instance().userExists(m_username)){
        QMessageBox::warning(this, "错误", "用户名已存在");
        return;
    }

    // 保存到本地数据库
    if(DatabaseManager::instance().addUser(m_username, password, m_email, m_phone, m_userType)){
        QMessageBox::information(this, "成功", "注册成功");
        accept();
    } else {
        QMessageBox::warning(this, "错误", "注册失败，请重试");
    }
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
