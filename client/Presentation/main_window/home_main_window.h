#ifndef HOMEMAINWINDOW_H
#define HOMEMAINWINDOW_H

#include <QMainWindow>
#include "Business/services/TicketService.h"
#include "Business/services/AuthService.h"

#include "Presentation/pages/ThanksPage/thanks_page.h"
#include "Presentation/pages/TicketPage/ticket_page.h"
#include "Presentation/pages/SettingPage/setting_page.h"
// #include "Presentation/pages/EquipmentPage/equipmentshow.h"

// #include "dialogs/vedio/mainwindow.h"
#include "Presentation/pages/LogPage/logmainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class HomeMainWindow; }
QT_END_NAMESPACE

class TicketPage;
class HomeMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    HomeMainWindow(const QString& currentUser, int currentUserType, QWidget *parent = nullptr);
    ~HomeMainWindow();

    void dealMenu();//

    // 设置工单服务
    void setTicketService(TicketService* ticketService);
    
    // 设置认证服务
    void setAuthService(AuthService* authService);

private slots:
    void on_btnNotif_clicked();

    void on_btnRecord_clicked();

private:
    TicketPage *ticketPage = nullptr;

    // MainWindow* notifWin_ = nullptr;

    //加载那几个切换页面
    void initPage();

private:
    Ui::HomeMainWindow *ui;

    ThanksWidget *m_thanksPage;
    TicketPage *m_ticketPage;
    SettingPage *m_settingPage;
    // EquipmentShow *m_equipmentPage;
    LogMainWindow *m_logPage;

    QString currentUser;
    int currentUserType;
    
    // 工单服务引用
    TicketService* ticketService_;
    
    // 认证服务引用
    AuthService* authService_;
    
    void updateGreeting();
};
#endif // HOMEMAINWINDOW_H
