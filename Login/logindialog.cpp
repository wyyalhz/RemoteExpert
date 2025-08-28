#include "logindialog.h"
#include "ui_logindialog.h"
#include "registerdialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog),
    currentUser(""),
    currentUserType(0)
{
    ui->setupUi(this);
    setWindowTitle("远程技术支持系统 - 登录");

    // 连接信号槽
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(ui->userTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoginDialog::onUserTypeChanged);

    // 设置用户类型选项
    ui->userTypeCombo->addItem("工厂用户");
    ui->userTypeCombo->addItem("技术专家");
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::onLoginClicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    int userType = ui->userTypeCombo->currentIndex();

    // 输入验证
    if(username.isEmpty() || password.isEmpty()){
        QMessageBox::warning(this, "错误", "用户名和密码不能为空");
        return;
    }

    if(username.length() < 3){
        QMessageBox::warning(this, "错误", "用户名长度至少3个字符");
        return;
    }

    // 本地验证
    if(DatabaseManager::instance().validateUser(username, password, userType)){
        QMessageBox::information(this, "成功", "登录成功");

        // 保存当前用户信息
        currentUser = username;
        currentUserType = userType;

        accept(); // 关闭对话框并返回Accepted
    } else {
        QMessageBox::warning(this, "错误", "用户名或密码错误");
    }
}

void LoginDialog::onRegisterClicked()
{
    RegisterDialog registerDialog(this);
    if(registerDialog.exec() == QDialog::Accepted){
        QMessageBox::information(this, "提示", "注册成功，请使用新账号登录");
        ui->usernameEdit->setText(registerDialog.getUsername());
        ui->passwordEdit->clear();
    }
}

void LoginDialog::onUserTypeChanged(int index)
{
    // 可以根据用户类型改变界面提示
    if(index == 0) {
        ui->titleLabel->setText("工厂用户登录");
    } else {
        ui->titleLabel->setText("技术专家登录");
    }
}
