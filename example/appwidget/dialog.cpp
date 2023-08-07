#include "dialog.h"
#include "./ui_dialog.h"
#include "dialog2.h"
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}


void Dialog::on_pushButton_clicked()
{
    ui->comboBox->setVisible(!ui->comboBox->isVisible());
}


void Dialog::on_pushButton_2_clicked(bool checked)
{
    Dialog2 dl2;
    dl2.exec();
}

