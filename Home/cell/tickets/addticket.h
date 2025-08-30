#ifndef ADDTICKET_H
#define ADDTICKET_H

#include <QWidget>

namespace Ui {
class AddTicket;
}

class AddTicket : public QWidget
{
    Q_OBJECT

public:
    explicit AddTicket(QString name, QWidget *parent = nullptr);
    ~AddTicket();

signals:
    void backRequested();

private slots:
    void on_btnAdd_clicked();

    void on_btnBack_clicked();

    void on_expert_textChanged(const QString &arg1);

    void on_title_textChanged(const QString &arg1);

    void on_description_textChanged();

private:
    Ui::AddTicket *ui;

    QString expert;
    QString title;
    QString description;

    QString factory;
    QString randomId();
};

#endif // ADDTICKET_H
