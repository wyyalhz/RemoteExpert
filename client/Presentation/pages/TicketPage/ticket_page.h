#ifndef WIDGET_H
#define WIDGET_H

#include "Presentation/dialogs/TicketDialog/ticket_dialog.h"
#include "Presentation/dialogs/TicketDialogDetail/ticket_dialog_detail.h"
#include "Business/services/TicketService.h"
#include "Business/services/AuthService.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class TicketPage; }
QT_END_NAMESPACE

class TicketPage : public QWidget
{
    Q_OBJECT

public:
    TicketPage(const QString& name, bool isExpert, QWidget *parent = nullptr);
    ~TicketPage();

    // 设置工单服务
    void setTicketService(TicketService* ticketService);
    
    // 设置认证服务（用于获取用户信息）
    void setAuthService(AuthService* authService);

private slots:
    void on_btnAdd_clicked();

    void searchTicket(bool isExpert, const QString& name);
    void createTicketDialog(const Ticket& ticket);

    void showTicketDetail(const QString& id);
    void returnToTicketList();
    void deleteTicket(const QString& id);
    
    // 工单服务响应处理
    void onTicketListReceived(const QList<Ticket>& tickets);
    void onTicketListFailed(const QString& error);
    void onTicketDetailReceived(const Ticket& ticket);
    void onTicketDetailFailed(const QString& error);
    void onTicketDeleted(int ticketId);
    void onTicketDeletedFailed(const QString& error);
    
    // 工单状态更新处理
    void onAcceptTicket(const QString& ticketId);
    void onRefuseTicket(const QString& ticketId);
    void onFinishTicket(const QString& ticketId);
    
    // 处理Connect信号，打开视频通话程序
    void onConnectRequest(const QString& ticketId);

private:
    Ui::TicketPage *ui;

    QStackedWidget *stackedWidget;
    TicketDialog *dialog;
    TicketDialogDetail *detail;
    
    // 当前显示的工单详情对话框引用
    TicketDialogDetail *currentDetailDialog_;

    const int spacing = 10;
    QString name;
    bool isExpert;
    
    // 工单服务引用
    TicketService* ticketService_;
    
    // 认证服务引用（用于获取用户信息）
    AuthService* authService_;
    
    // 显示加载状态
    void showLoading(bool loading);
    
    // 获取当前用户ID
    int getCurrentUserId() const;
};
#endif // WIDGET_H
