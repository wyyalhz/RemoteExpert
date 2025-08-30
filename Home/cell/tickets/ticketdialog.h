#ifndef TICKETDIALOG_H
#define TICKETDIALOG_H

#include <QWidget>

namespace Ui {
class TicketDialog;
}

class TicketDialog : public QWidget
{
    Q_OBJECT

public:
    explicit TicketDialog(bool isExpert, QWidget *parent = nullptr);
    ~TicketDialog();

    void setTicket(const QString& id, const QString &title);

signals:
    void enterRequested(const QString& id);

    void deleteRequested(const QString& id);

private slots:
    void on_btnEnter_clicked();

    void on_btnDelete_clicked();

private:
    Ui::TicketDialog *ui;

    QString ticketId;
};

#endif // TICKETDIALOG_H
