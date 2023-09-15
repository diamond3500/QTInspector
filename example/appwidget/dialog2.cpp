#include "dialog2.h"
#include "ui_dialog2.h"
#include <QTimer>

Dialog2::Dialog2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog2)
{
    ui->setupUi(this);
}

Dialog2::~Dialog2()
{
    delete ui;
}

void Dialog2::on_pushButton_clicked()
{
    QTimer* t = new QTimer(this);
    t->setInterval(1000);
    t->start();
}

