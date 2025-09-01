#include "ticket_dialog.h"
#include "ui_ticket_dialog.h"

TicketDialog::TicketDialog(bool isExpert, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TicketDialog)
{
    ui->setupUi(this);
    ui->btnDelete->setVisible(!isExpert);
    layout()->activate();
}

TicketDialog::~TicketDialog()
{
    delete ui;
}

void TicketDialog::setTicket(const QString &id, const QString &title){
    ticketId = id;
    ui->id->setText(id);
    ui->title->setText(title);
}

void TicketDialog::on_btnEnter_clicked()
{
    emit enterRequested(ticketId);
}

void TicketDialog::on_btnDelete_clicked()
{
    emit deleteRequested(ticketId);
}
