#include <QPushButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QDebug>
#include "tickets/ticketpage.h"
#include "homemainwindow.h"
#include "ui_homemainwindow.h"

HomeMainWindow::HomeMainWindow(const QString& currentUser, int currentUserType, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HomeMainWindow)
    , m_thanksPage(nullptr)//指针置空
    , m_ticketPage(nullptr)
    , currentUser(currentUser)
    , currentUserType(currentUserType)
{
    ui->setupUi(this);

    initPage();
}

HomeMainWindow::~HomeMainWindow()
{
    delete ui;
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

    //把这些页面加到stackWidget
    ui->stackWidget->addWidget(m_thanksPage);
    ui->stackWidget->addWidget(m_ticketPage);
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
    }while(false);
}

