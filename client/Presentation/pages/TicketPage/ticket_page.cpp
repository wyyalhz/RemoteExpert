#include "ticket_page.h"
#include "ui_ticket_page.h"
#include "Presentation/dialogs/TicketDialog/ticket_dialog.h"
#include "Presentation/dialogs/TicketDialogDetail/ticket_dialog_detail.h"
#include "Presentation/dialogs/AddTicket/add_ticket.h"
#include <QGridLayout>
#include <QMessageBox>

#include <QDebug>

TicketPage::TicketPage(const QString& name, bool isExpert, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TicketPage)
    , name(name)
    , isExpert(isExpert)
    , ticketService_(nullptr)
{
    ui->setupUi(this);
    ui->btnAdd->setVisible(!isExpert);
    layout()->activate();

    // 初始时不自动搜索，等待TicketService设置后再搜索
}

TicketPage::~TicketPage()
{
    delete ui;
}

void TicketPage::setTicketService(TicketService* ticketService)
{
    ticketService_ = ticketService;
    
    if (ticketService_) {
        // 连接工单服务的信号
        connect(ticketService_, &TicketService::ticketListReceived, 
                this, &TicketPage::onTicketListReceived);
        connect(ticketService_, &TicketService::ticketListFailed, 
                this, &TicketPage::onTicketListFailed);
        connect(ticketService_, &TicketService::ticketDeleted, 
                this, &TicketPage::onTicketDeleted);
        connect(ticketService_, &TicketService::ticketDeletedFailed, 
                this, &TicketPage::onTicketDeletedFailed);
        
        // 设置完成后开始搜索工单
        searchTicket(isExpert, name);
    }
}

void TicketPage::searchTicket(bool isExpert, const QString& name){
    if (!ticketService_) {
        qDebug() << "TicketService not initialized";
        return;
    }
    
    showLoading(true);
    ui->ticketListWidget->clear();
    
    // 根据用户类型和名称获取工单列表
    if (!isExpert) {
        // 工厂用户：获取自己创建的工单
        ticketService_->getTicketsByCreator(0); // TODO: 需要传入实际的用户ID
    } else {
        // 技术专家：获取分配给自己的工单
        ticketService_->getTicketsByAssignee(0); // TODO: 需要传入实际的用户ID
    }
}

void TicketPage::onTicketListReceived(const QList<Ticket>& tickets)
{
    showLoading(false);
    ui->ticketListWidget->clear();
    
    for (const Ticket& ticket : tickets) {
        createTicketDialog(ticket.getTicketId());
    }
}

void TicketPage::onTicketListFailed(const QString& error)
{
    showLoading(false);
    QMessageBox::warning(this, "错误", QString("获取工单列表失败: %1").arg(error));
}

void TicketPage::createTicketDialog(const QString& id){
    if (!ticketService_) {
        return;
    }
    
    // 获取工单详情
    Ticket ticket = ticketService_->getTicketByTicketId(id);
    if (ticket.getTicketId().isEmpty()) {
        return;
    }

    TicketDialog *dialog = new TicketDialog(isExpert, this);
    dialog->setTicket(id, ticket.getTitle());
    dialog->setWindowFlags(Qt::Widget);

    connect(dialog, &TicketDialog::enterRequested, this, &TicketPage::showTicketDetail);
    connect(dialog, &TicketDialog::deleteRequested, this, &TicketPage::deleteTicket);

    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(dialog->sizeHint());
    ui->ticketListWidget->addItem(item);
    ui->ticketListWidget->setItemWidget(item, dialog);
    ui->ticketListWidget->setSpacing(3);
}

void TicketPage::on_btnAdd_clicked()
{
    if (isExpert){
        return;
    }
    this->hide();
    AddTicket *newTicket = new AddTicket(name);
    connect(newTicket, &AddTicket::backRequested, this, &TicketPage::returnToTicketList);
    newTicket->show();
}

void TicketPage::deleteTicket(const QString& id){
    if (!ticketService_) {
        QMessageBox::warning(this, "错误", "工单服务未初始化");
        return;
    }
    
    showLoading(true);
    // 将字符串ID转换为整数ID（TODO: 需要确认ID格式）
    bool ok;
    int ticketId = id.toInt(&ok);
    if (ok) {
        ticketService_->deleteTicket(ticketId);
    } else {
        showLoading(false);
        QMessageBox::warning(this, "错误", "无效的工单ID");
    }
}

void TicketPage::onTicketDeleted(int ticketId)
{
    showLoading(false);
    QMessageBox::information(this, "成功", "工单删除成功");
    searchTicket(isExpert, name); // 刷新列表
}

void TicketPage::onTicketDeletedFailed(const QString& error)
{
    showLoading(false);
    QMessageBox::warning(this, "错误", QString("删除工单失败: %1").arg(error));
}

void TicketPage::showTicketDetail(const QString& id){
    if (!ticketService_) {
        QMessageBox::warning(this, "错误", "工单服务未初始化");
        return;
    }
    
    this->hide();
    TicketDialogDetail *detailDialog = new TicketDialogDetail(isExpert);
    connect(detailDialog, &TicketDialogDetail::backRequest, this, &TicketPage::returnToTicketList);

    // 获取工单详情
    Ticket ticket = ticketService_->getTicketByTicketId(id);
    if (!ticket.getTicketId().isEmpty()) {
        detailDialog->setTicketData(
            id,
            ticket.getStatus(),
            ticket.getCreatorName(), // TODO: 需要确认字段名
            ticket.getAssigneeName(), // TODO: 需要确认字段名
            ticket.getTitle(),
            ticket.getDescription()
        );
    }

    detailDialog->show();
}

void TicketPage::returnToTicketList(){
    this->show();
    searchTicket(isExpert, name);
}

void TicketPage::showLoading(bool loading)
{
    ui->btnAdd->setEnabled(!loading);
    // 可以添加其他UI元素的加载状态控制
}
