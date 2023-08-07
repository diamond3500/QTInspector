#include "show_uri_content.h"
#include "ui_show_uri_content.h"

ShowUriContent::ShowUriContent(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShowUriContent)
{
    ui->setupUi(this);
  HideAll();
}

ShowUriContent::~ShowUriContent()
{
    delete ui;
}

void ShowUriContent::SetContent(const pb::GetUriContentRsp& rsp) {
    HideAll();
    QString uri = QString::fromStdString(rsp.uri());
    if (uri.isEmpty()) {
      return;
    }
    if (uri.endsWith(".png")) {
      ShowImage(rsp.content());
    } else {
      ShowText(rsp.content());
    }
}



void ShowUriContent::ShowImage(const std::string& buffer) {
    QPixmap pixmap;
    pixmap.loadFromData((const uchar*)buffer.data(), (uint)buffer.length());
    ui->show_image->show();
    ui->show_image->setPixmap(pixmap);
    ui->show_image->setScaledContents(true);
}

void ShowUriContent::ShowText(const std::string& buffer) {
    ui->show_text->show();
    ui->show_text->clear();
    ui->show_text->setText(QString::fromStdString(buffer));
}

void ShowUriContent::HideAll() {
    ui->show_image->hide();
    ui->show_text->hide();
}