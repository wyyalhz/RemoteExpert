#ifndef THANKSWIDGET_H
#define THANKSWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ThanksWidget; }
QT_END_NAMESPACE

class ThanksWidget : public QWidget
{
    Q_OBJECT

public:
    ThanksWidget(QWidget *parent = nullptr);
    ~ThanksWidget();

private:
    Ui::ThanksWidget *ui;
};
#endif // THANKSWIDGET_H
