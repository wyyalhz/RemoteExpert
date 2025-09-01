#ifndef WIDGET_H
#define WIDGET_H

#include "Presentation/dialogs/TicketDialog/ticket_dialog.h"
#include "Presentation/dialogs/TicketDialogDetail/ticket_dialog_detail.h"
#include "Business/services/TicketService.h"
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

private slots:
    void on_btnAdd_clicked();

    void searchTicket(bool isExpert, const QString& name);
    void createTicketDialog(const QString& id);

    void showTicketDetail(const QString& id);
    void returnToTicketList();
    void deleteTicket(const QString& id);
    
    // 工单服务响应处理
    void onTicketListReceived(const QList<Ticket>& tickets);
    void onTicketListFailed(const QString& error);
    void onTicketDeleted(int ticketId);
    void onTicketDeletedFailed(const QString& error);

private:
    Ui::TicketPage *ui;

    QStackedWidget *stackedWidget;
    TicketDialog *dialog;
    TicketDialogDetail *detail;

    const int spacing = 10;
    QString name;
    bool isExpert;
    
    // 工单服务引用
    TicketService* ticketService_;
    
    // 显示加载状态
    void showLoading(bool loading);
};
#endif // WIDGET_H
