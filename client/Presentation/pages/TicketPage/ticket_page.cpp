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
    , authService_(nullptr)
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
        
        // 延迟获取工单列表，确保用户信息已经设置完成
        QTimer::singleShot(100, this, [this]() {
            if (authService_ && authService_->isLoggedIn()) {
                searchTicket(isExpert, name);
            }
        });
    }
}

void TicketPage::setAuthService(AuthService* authService)
{
    authService_ = authService;
}

int TicketPage::getCurrentUserId() const
{
    if (!authService_) {
        qDebug() << "AuthService is null";
        return -1;
    }
    
    if (!authService_->isLoggedIn()) {
        qDebug() << "User is not logged in";
        return -1;
    }
    
    User currentUser = authService_->getCurrentUser();
    int userId = currentUser.getId();
    
    qDebug() << "Current user ID:" << userId << "Username:" << currentUser.getUsername();
    
    if (userId == -1) {
        qDebug() << "User ID is -1, this might indicate incomplete user data";
    }
    
    return userId;
}

void TicketPage::searchTicket(bool isExpert, const QString& name){
    if (!ticketService_) {
        qDebug() << "TicketService not initialized";
        return;
    }
    
    showLoading(true);
    ui->ticketListWidget->clear();
    
    // 获取当前用户ID
    int userId = getCurrentUserId();
    qDebug() << "Searching tickets for user ID:" << userId;
    
    if (userId == -1) {
        showLoading(false);
        qDebug() << "Cannot get user ID, skipping ticket search";
        // 不显示错误对话框，只是跳过搜索
        return;
    }
    
    // 根据用户类型和名称获取工单列表
    if (!isExpert) {
        // 工厂用户：获取自己创建的工单
        ticketService_->getTicketsByCreator(userId);
    } else {
        // 技术专家：获取分配给自己的工单
        ticketService_->getTicketsByAssignee(userId);
    }
}

void TicketPage::onTicketListReceived(const QList<Ticket>& tickets)
{
    showLoading(false);
    ui->ticketListWidget->clear();
    
    for (const Ticket& ticket : tickets) {
        createTicketDialog(ticket);
    }
}

void TicketPage::onTicketListFailed(const QString& error)
{
    showLoading(false);
    QMessageBox::warning(this, "错误", QString("获取工单列表失败: %1").arg(error));
}

void TicketPage::createTicketDialog(const Ticket& ticket){
    if (!ticketService_) {
        return;
    }
    
    // 检查工单是否有效
    if (!ticket.isValid()) {
        return;
    }

    TicketDialog *dialog = new TicketDialog(isExpert, this);
    dialog->setTicket(ticket.getTicketId(), ticket.getTitle());
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
    
    // 设置TicketService到AddTicket
    if (ticketService_) {
        newTicket->setTicketService(ticketService_);
    }
    
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

    // TODO: 这里需要从缓存或重新获取工单详情
    // 暂时使用空数据，后续可以优化为从缓存获取
    detailDialog->setTicketData(
        id,
        "open", // 默认状态
        "",     // 创建者名称
        "",     // 分配者名称
        "工单详情", // 标题
        ""      // 描述
    );

    detailDialog->show();
}

void TicketPage::returnToTicketList(){
    this->show();
    searchTicket(isExpert, name);
}

void TicketPage::showLoading(bool loading)
{
    // 下面这条语句可能会造成工厂端创建工单按钮被异常禁用，因此暂时注释掉，后续需要优化
    // ui->btnAdd->setEnabled(!loading);
    // 可以添加其他UI元素的加载状态控制
}
