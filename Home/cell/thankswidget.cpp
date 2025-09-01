#include "thankswidget.h"
#include "ui_thankswidget.h"

ThanksWidget::ThanksWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ThanksWidget)
{
    ui->setupUi(this);
}

ThanksWidget::~ThanksWidget()
{
    delete ui;
}

