#include "ticket_dialog_detail.h"
#include "ui_ticket_dialog_detail.h"
#include "ui_mainwindow.h"
#include "../Presentation/dialogs/vedio/mainwindow.h"

TicketDialogDetail::TicketDialogDetail(bool isExpert, QWidget *parent) :
    QWidget(parent)
    , ui(new Ui::TicketDialogDetail)
    , isExpert(isExpert)
{
    ui->setupUi(this);
    ui->btnAccept->setVisible(isExpert);
    ui->btnRefuse->setVisible(isExpert);
    ui->btnFinish->setVisible(!isExpert);
    ui->btnConnect->setEnabled(false);
    layout()->activate();
}

TicketDialogDetail::~TicketDialogDetail()
{
    delete ui;
}

void TicketDialogDetail::setTicketData(const QString &id, const QString &status,
                                      const QString &factory, const QString &expert,
                                      const QString &title, const QString &description)
{
    ticketId = id;
    ui->id->setText(id);
    ui->status->setText(status);
    ui->factory->setText(factory);
    ui->expert->setText(expert);
    ui->title->setText(title);
    ui->description->setText(description);
    if (status == "processing") {
        ui->btnConnect->setEnabled(true);
    } else {
        ui->btnConnect->setEnabled(false);
    }
}

void TicketDialogDetail::on_btnConnect_clicked()
{
    //this->hide();
    //emit connect(ticketId);

//    MainWindow w;
//    w.show();

    if (!notifWin_) {
        notifWin_ = new MainWindow(this);
        // notifWin_->setAttribute(Qt::WA_DeleteOnClose, false); // 可选
    }
    notifWin_->show();
    notifWin_->raise();
    notifWin_->activateWindow();
}

void TicketDialogDetail::on_btnBack_clicked()
{
    emit backRequest();
    ui->btnConnect->setEnabled(false);
    this->close();
}

void TicketDialogDetail::on_btnAccept_clicked()
{
    emit acceptProcess(ticketId);
    ui->btnConnect->setEnabled(true);
}

void TicketDialogDetail::on_btnRefuse_clicked()
{
    emit refuseProcess(ticketId);
    on_btnBack_clicked();
}
void TicketDialogDetail::on_btnFinish_clicked()
{
    emit finishProcess(ticketId);
    ui->btnConnect->setEnabled(false);
}
