#ifndef TICKETDIALOGDETAIL_H
#define TICKETDIALOGDETAIL_H

#include <QWidget>
#include "Presentation/dialogs/vedio/mainwindow.h"

namespace Ui {
class TicketDialogDetail;
}

class TicketDialogDetail : public QWidget
{
    Q_OBJECT

public:
    explicit TicketDialogDetail(bool isExpert, QWidget *parent = nullptr);
    ~TicketDialogDetail();

    void setTicketData(const QString& id, const QString &status,
                       const QString &factory, const QString &expert,
                       const QString &title, const QString &description);
//    MainWindow* notifWin_ = nullptr;
signals:
    void backRequest();

    void connect(const QString& id);

    void acceptProcess(const QString& id);

    void refuseProcess(const QString& id);

    void finishProcess(const QString& id);

private slots:
    void on_btnConnect_clicked();

    void on_btnBack_clicked();

    void on_btnAccept_clicked();

    void on_btnRefuse_clicked();

    void on_btnFinish_clicked();

private:
    Ui::TicketDialogDetail *ui;

    MainWindow* notifWin_ = nullptr;

    QString ticketId;
    bool isExpert;
};

#endif // TICKETDIALOGDETAIL_H
