#include "login_dialog.h"
#include "ui_login_dialog.h"
#include "Presentation/dialogs/RegisterDialog/register_dialog.h"
#include <QSvgRenderer>
#include <QPainter>

#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QDesktopServices>

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

    // 高DPI图标锐利
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 无边框 + 透明背景（结合样式表圆角）
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
//    setAttribute(Qt::WA_TranslucentBackground);

    // 给中心内容加「真实阴影」，不要直接加到Dialog本体
    auto shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(0, 0, 0, 70));
    ui->center_widget->setGraphicsEffect(shadow);

    // 顶部工具按钮：关闭/最小化（你现在两个按钮可自定义用途，这里示例关闭+打开GitHub）
    connect(ui->toolButton_close,   &QToolButton::clicked, this, &QWidget::close);

    // 如果你想做最小化，把 toolButton_2 当成最小化按钮：
    // connect(ui->toolButton_2, &QToolButton::clicked, this, [this]{ setWindowState(Qt::WindowMinimized); });

//    // 你现在给 toolButton_2 放的是 github.svg，如果想真的打开链接，可启用下面两行：
//     connect(ui->toolButton_github, &QToolButton::clicked, this, []{
//         QDesktopServices::openUrl(QUrl("https://github.com/wyyalhz/RemoteExpert"));
//     });
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

    // 兼容：优先用 QCheckBox#checkAgree；没有的话退回 QRadioButton#radioButton
    bool agreed = ui->LogAgreeButton->isChecked();

    if (!agreed) {
        QMessageBox::warning(this, tr("提示"),
                             tr("请先同意服务协议和隐私指导！"));
        return; // 拦截，不继续登录流程
    }

    // ✓ 已勾选，继续你的登录逻辑：
//     doLogin();
//     accept();  // 如果这是个对话框且登录成功

//    // 本地验证
//    if(DatabaseManager::instance().validateUser(username, password, userType)){
//        QMessageBox::information(this, "成功", "登录成功");

//        // 保存当前用户信息
//        currentUser = username;
//        currentUserType = userType;

//        accept(); // 关闭对话框并返回Accepted
//    } else {
//        QMessageBox::warning(this, "错误", "用户名或密码错误");
//    }
    accept();

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

    //根据用户选择改变图标
    QString path = (index == 0)
            ? QStringLiteral(":/login/gear-wide-connected.svg")
            : QStringLiteral(":/login/wrench-adjustable-circle.svg");

    QSvgRenderer renderer(path);

    // 根据 Label 的大小生成 Pixmap
    QSize target = ui->label_profile->size() * 0.6;
    QPixmap pm(target);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    renderer.render(&p);

    ui->label_profile->setPixmap(pm);
    ui->label_profile->setScaledContents(false); // 关键：不要让 Label 再拉伸

}

void LoginDialog::on_titleLabel_linkActivated(const QString &link)
{

}

//ui
// 拖动窗口（按住 title_widget 区域拖动）
void LoginDialog::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        // 只允许在标题区域开始拖动，避免误触控件
        const QPoint gPos = mapToGlobal(e->pos());
        const QRect  titleRect = ui->title_widget->geometry();
        if (titleRect.contains(e->pos())) {
            m_dragging = true;
            m_dragStart = gPos - frameGeometry().topLeft();
        }
    }
    QDialog::mousePressEvent(e);
}

void LoginDialog::mouseMoveEvent(QMouseEvent *e) {
    if (m_dragging && (e->buttons() & Qt::LeftButton)) {
        const QPoint gPos = mapToGlobal(e->pos());
        move(gPos - m_dragStart);
    }
    QDialog::mouseMoveEvent(e);
}

//// 释放时结束拖动（可选）
//void LoginDialog::mouseReleaseEvent(QMouseEvent *e) {
//    m_dragging = false;
//    QDialog::mouseReleaseEvent(e);
//}

void LoginDialog::on_usernameEdit_textChanged(const QString &arg1)
{
    currentUser = arg1;
}

void LoginDialog::on_userTypeCombo_currentIndexChanged(int index)
{
    currentUserType = index;
}
