#include "add_ticket.h"
#include "ui_add_ticket.h"
#include <random>
#include <QMessageBox>

AddTicket::AddTicket(QString factory, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddTicket),
    factory(factory),
    ticketService_(nullptr)
{
    ui->setupUi(this);
}

AddTicket::~AddTicket()
{
    delete ui;
}

void AddTicket::setTicketService(TicketService* ticketService)
{
    ticketService_ = ticketService;
    
    if (ticketService_) {
        // 连接工单服务的信号
        connect(ticketService_, &TicketService::ticketCreated, 
                this, &AddTicket::onTicketCreated);
        connect(ticketService_, &TicketService::ticketCreatedFailed, 
                this, &AddTicket::onTicketCreatedFailed);
    }
}

void AddTicket::on_btnAdd_clicked()
{
    if (!ticketService_) {
        QMessageBox::warning(this, "错误", "工单服务未初始化");
        return;
    }
    
    // 验证输入
    if (title.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入工单标题");
        return;
    }
    
    if (description.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入工单描述");
        return;
    }
    
    if (expert.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入技术专家");
        return;
    }
    
    showLoading(true);
    
    // 通过工单服务创建工单
    bool success = ticketService_->createTicket(
        title, 
        description, 
        "normal", // 默认优先级
        "general", // 默认分类
        expert, // 专家用户名
        QJsonObject() // 设备信息（暂时为空）
    );
    
    if (!success) {
        showLoading(false);
        QMessageBox::warning(this, "错误", ticketService_->getLastError());
    }
}

void AddTicket::onTicketCreated(const Ticket& ticket)
{
    showLoading(false);
    QMessageBox::information(this, "成功", "工单创建成功！");
    on_btnBack_clicked();
}

void AddTicket::onTicketCreatedFailed(const QString& error)
{
    showLoading(false);
    QMessageBox::warning(this, "错误", QString("创建工单失败: %1").arg(error));
}

void AddTicket::on_btnBack_clicked()
{
    emit backRequested();
    this->close();
}

void AddTicket::on_expert_textChanged(const QString &arg1)
{
    expert = arg1;
}

void AddTicket::on_title_textChanged(const QString &arg1)
{
    title = arg1;
}

void AddTicket::on_description_textChanged()
{
    description = ui->description->toPlainText();
}

void AddTicket::showLoading(bool loading)
{
    ui->btnAdd->setEnabled(!loading);
    ui->btnBack->setEnabled(!loading);
    ui->expert->setEnabled(!loading);
    ui->title->setEnabled(!loading);
    ui->description->setEnabled(!loading);
    
    if (loading) {
        ui->btnAdd->setText("创建中...");
    } else {
        ui->btnAdd->setText("创建工单");
    }
}
