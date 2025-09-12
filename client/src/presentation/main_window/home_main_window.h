#ifndef HOMEMAINWINDOW_H
#define HOMEMAINWINDOW_H

#include <QMainWindow>
#include "../../business/services/ticket_service.h"
#include "../../business/services/auth_service.h"

#include "../pages/thanks_page/thanks_page.h"
#include "../pages/ticket_page/ticket_page.h"
#include "../pages/setting_page/setting_page.h"
// #include "../pages/equipment_page/equipment_show.h"

// #include "dialogs/vedio/mainwindow.h"
#include "../pages/log_page/log_main_window.h"
#include "../dialogs/equipment_dialog/equipment_show.h"

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
    EquipmentShow *m_equipmentPage;
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
