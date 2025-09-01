#include "add_ticket.h"
#include "ui_add_ticket.h"
#include <random>
#include <QSqlQuery>

AddTicket::AddTicket(QString factory, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddTicket),
    factory(factory)
{
    ui->setupUi(this);
}

AddTicket::~AddTicket()
{
    delete ui;
}

QString AddTicket::randomId(){
    std::random_device rd;
    std::mt19937 gen;
    if (rd.entropy() == 0) {
        gen.seed(std::time(nullptr));
    } else {
        gen.seed(rd());
    }
    std::uniform_int_distribution<> distrib(0, 9);

    QString result;
    for (int i = 0; i < 10; ++i) {
        result.append(QString::number(distrib(gen)));
    }

    QSqlQuery query;
    query.prepare("select * from tickets where sid = :id");
    query.bindValue(":id", result);
    query.exec();

    if (query.next()){
        return randomId();
    } else {
        return result;
    }
}

void AddTicket::on_btnAdd_clicked()
{
    QSqlQuery query;
    query.prepare("insert into tickets (sid, factory, expert, status, title, description) "
                  "values (:sid, :factory, :expert, :status, :title, :description)");
    query.bindValue(":sid", randomId());
    query.bindValue(":factory", factory);
    query.bindValue(":expert", expert);
    query.bindValue(":status", "open");
    query.bindValue(":title", title);
    query.bindValue(":description", description);
    query.exec();
    on_btnBack_clicked();
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
