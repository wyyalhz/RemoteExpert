#include "ticket_page.h"
#include "ui_ticket_page.h"
#include "Presentation/dialogs/TicketDialog/ticket_dialog.h"
#include "Presentation/dialogs/TicketDialogDetail/ticket_dialog_detail.h"
#include "Presentation/dialogs/AddTicket/add_ticket.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QGridLayout>

#include <QDebug>

TicketPage::TicketPage(const QString& name, bool isExpert, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TicketPage)
    , name(name)
    , isExpert(isExpert)
{
    ui->setupUi(this);
    ui->btnAdd->setVisible(!isExpert);
    layout()->activate();

    searchTicket(isExpert, name);
}

TicketPage::~TicketPage()
{
    delete ui;
}

void TicketPage::searchTicket(bool isExpert, const QString& name){
    ui->ticketListWidget->clear();
    QString sql;
    if (!isExpert) {
        sql = "select sid from tickets where factory = '" + name + "'";
    } else {
        sql = "select sid from tickets where expert = '" + name + "'";
    }
    QSqlQuery query;
    query.exec(sql);
    while(query.next()){
        createTicketDialog(query.record().value("sid").toString());
    }
}

void TicketPage::createTicketDialog(const QString& id){
    QSqlQuery query("select title from tickets where sid = '" + id + "'");
    if (!query.exec() || !query.next()) {
        return;
    }

    TicketDialog *dialog = new TicketDialog(isExpert, this);
    dialog->setTicket(id, query.value("title").toString());
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
    QSqlQuery query;
    query.prepare("delete from tickets where sid = :id");
    query.bindValue(":id", id);
    query.exec();
    searchTicket(isExpert, name);
}

void TicketPage::showTicketDetail(const QString& id){
    this->hide();
    TicketDialogDetail *detailDialog = new TicketDialogDetail(isExpert);
    connect(detailDialog, &TicketDialogDetail::backRequest, this, &TicketPage::returnToTicketList);

    QSqlQuery query;
    query.prepare("select status, factory, expert, title, description from tickets where sid = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        detailDialog->setTicketData(
            id,
            query.value("status").toString(),
            query.value("factory").toString(),
            query.value("expert").toString(),
            query.value("title").toString(),
            query.value("description").toString()
        );
    }

    detailDialog->show();
}

void TicketPage::returnToTicketList(){
    this->show();
    searchTicket(isExpert, name);
}
