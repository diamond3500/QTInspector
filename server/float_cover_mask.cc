#include "float_cover_mask.h"
#include "ui_float_cover_mask.h"

FloatCoverMask::FloatCoverMask(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FloatCoverMask)
{
    ui->setupUi(this);
}

FloatCoverMask::~FloatCoverMask()
{
    delete ui;
}
