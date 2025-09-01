#include <QPushButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include "Presentation/pages/TicketPage/ticket_page.h"
#include "Presentation/pages/ThanksPage/thanks_page.h"
#include "Presentation/pages/SettingPage/setting_page.h"
#include "home_main_window.h"
#include "ui_home_main_window.h"
#include "dialogs/vedio/mainwindow.h"

HomeMainWindow::HomeMainWindow(const QString& currentUser, int currentUserType, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HomeMainWindow)
    , m_thanksPage(nullptr)//指针置空
    , m_ticketPage(nullptr)
    , m_settingPage(nullptr)
    , m_equipmentPage(nullptr)
    , currentUser(currentUser)
    , currentUserType(currentUserType)
    , ticketService_(nullptr)
    , authService_(nullptr)
{
    ui->setupUi(this);

    // 初始化问候语
    updateGreeting();

    // 可选：定时刷新（比如每分钟刷新一次）
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &HomeMainWindow::updateGreeting);
    timer->start(60000); // 60秒刷新一次

    initPage();
}

HomeMainWindow::~HomeMainWindow()
{
    delete ui;
}

void HomeMainWindow::setTicketService(TicketService* ticketService)
{
    ticketService_ = ticketService;
    
    // 如果TicketPage已经创建，设置TicketService
    if (m_ticketPage) {
        m_ticketPage->setTicketService(ticketService);
    }
}

void HomeMainWindow::setAuthService(AuthService* authService)
{
    authService_ = authService;
    
    // 如果TicketPage已经创建，设置AuthService
    if (m_ticketPage) {
        m_ticketPage->setAuthService(authService);
    }
}

// 设置问候语
void HomeMainWindow::updateGreeting() {
    int hour = QTime::currentTime().hour();
    QString greeting;

    if (hour >= 6 && hour < 12) {
        greeting = "上午好!";
    } else if (hour >= 12 && hour < 18) {
        greeting = "下午好!";
    } else if (hour >= 18 && hour < 23) {
        greeting = "晚上好!";
    } else {
        greeting = "夜深了..";
    }

    ui->titleLabel_1->setText(greeting);

    if(currentUserType == 0){
        ui->titleLabel_2->setText("工厂用户:");
    }else{
        ui->titleLabel_2->setText("技术专家:");
    }

    ui->titleLabel_3->setText(currentUser);
}


void HomeMainWindow::initPage()
{
    m_thanksPage = new ThanksWidget(this);//加上this就不用析构了
    qDebug() << currentUserType;
    if (currentUserType == 1) {
        m_ticketPage = new TicketPage(currentUser, true, this);
    } else {
        m_ticketPage = new TicketPage(currentUser, false, this);
    }
    m_settingPage = new SettingPage(this);
    m_equipmentPage = new EquipmentShow(this);
    QString portName = "ttyS1";
    qint32 baudRate = 9600;
    m_equipmentPage->openSerialPort(portName, baudRate);
    //JSON Style
    /*
    {"sensors":[{"name":"Pressure","value":12.7,"unit":"MPa"},{"name":"Temperature","value":47.9,"unit":"\u00B0C"}],"logs":[{"level":"INFO","message":"Heartbeat OK"}],"faults":[{"code":"F1203","description":"Pressure spike","severity":"HIGH"}]}
    {"sensors":[{"name":"Pressure","value":9.85,"unit":"MPa"},{"name":"Temperature","value":51.3,"unit":"\u00B0C"}]}
    {"sensors":[{"name":"Pressure","value":15.2,"unit":"MPa"},{"name":"Temperature","value":62.1,"unit":"\u00B0C"}],"logs":[{"level":"WARNING","message":"sensor drift detected"}]}
    {"faults":[{"code":"F9012","description":"Temp probe open circuit","severity":"CRITICAL"}]}
    */




    // 如果TicketService已经设置，立即设置到TicketPage
    if (ticketService_) {
        m_ticketPage->setTicketService(ticketService_);
    }
    
    // 如果AuthService已经设置，立即设置到TicketPage
    if (authService_) {
        m_ticketPage->setAuthService(authService_);
    }

    //把这些页面加到stackWidget
    ui->stackWidget->addWidget(m_thanksPage);
    ui->stackWidget->addWidget(m_ticketPage);
    ui->stackWidget->addWidget(m_settingPage);
    ui->stackWidget->addWidget(m_equipmentPage);
    ui->stackWidget->setCurrentIndex(0);

    //实现按钮切换页面
    auto l = ui->side_nav->children();
    for(auto it:l){
        if(it->objectName().contains("btn"))
        {
            connect(static_cast<QToolButton*>(it), &QToolButton::clicked, this, &HomeMainWindow::dealMenu);
        }
    }
}

void HomeMainWindow::dealMenu(){
    auto str = sender()->objectName();
    do{
        if("btnThanks" == str){
            ui->stackWidget->setCurrentIndex(0);
            break;
        }
        if("btnOrders" == str){
            ui->stackWidget->setCurrentIndex(1);
            break;
        }
        if("btnSettings" == str){
            ui->stackWidget->setCurrentIndex(2);
            break;
        }
        if("btnDevices" == str){
            ui->stackWidget->setCurrentIndex(3);
            break;
        }
    }while(false);
}



//temp test
void HomeMainWindow::on_btnNotif_clicked()
{
//    if (!notifWin_) {
//        notifWin_ = new MainWindow(this);
//        // notifWin_->setAttribute(Qt::WA_DeleteOnClose, false); // 可选
//    }
//    notifWin_->show();
//    notifWin_->raise();
//    notifWin_->activateWindow();
}
