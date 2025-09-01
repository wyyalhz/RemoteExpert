#ifndef ADDTICKET_H
#define ADDTICKET_H

#include <QWidget>
#include "Business/services/TicketService.h"

namespace Ui {
class AddTicket;
}

class AddTicket : public QWidget
{
    Q_OBJECT

public:
    explicit AddTicket(QString name, QWidget *parent = nullptr);
    ~AddTicket();

    // 设置工单服务
    void setTicketService(TicketService* ticketService);

signals:
    void backRequested();

private slots:
    void on_btnAdd_clicked();

    void on_btnBack_clicked();

    void on_expert_textChanged(const QString &arg1);

    void on_title_textChanged(const QString &arg1);

    void on_description_textChanged();
    
    // 工单服务响应处理
    void onTicketCreated(const Ticket& ticket);
    void onTicketCreatedFailed(const QString& error);

private:
    Ui::AddTicket *ui;

    QString expert;
    QString title;
    QString description;

    QString factory;
    
    // 工单服务引用
    TicketService* ticketService_;
    
    // 显示加载状态
    void showLoading(bool loading);
};

#endif // ADDTICKET_H
