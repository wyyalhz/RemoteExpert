#include "thanks_page.h"
#include "ui_thanks_page.h"

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

