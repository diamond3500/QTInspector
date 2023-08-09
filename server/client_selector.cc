#include "client_selector.h"
#include "ui_client_selector.h"

ClientSelector::ClientSelector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientSelector)
{
    ui->setupUi(this);
  ui->client_list_->setIconSize(QSize(400, 400));
}

ClientSelector::~ClientSelector()
{
    delete ui;
}

void ClientSelector::UpdateClient(TcpClientImpl* client, const pb::AppInfoRsp& app_info) {
    QListWidgetItem* newItem = FindWidgetItem(client);
    if (newItem == nullptr) {
      newItem = new QListWidgetItem(ui->client_list_);
      newItem->setData(Qt::UserRole + 1, (qint64)client);
      ui->client_list_->insertItem(ui->client_list_->count(), newItem);
    }
    newItem->setText(QString::fromStdString(app_info.appname()));
    if (!app_info.windows().empty()) {
      QPixmap image;
      auto& screen_image_data = app_info.windows(0).screenimage();
      QByteArray object_byte_array(screen_image_data.data(),
                                   (int)screen_image_data.size());
      QDataStream data_stream(object_byte_array);
      data_stream >> image;
      if (!image.isNull()) {
        QIcon icon;
        icon.addPixmap(image);
        newItem->setIcon(icon);
      }
    }
}

void ClientSelector::OnRemoveClient(TcpClientImpl* client) {
    QListWidgetItem* newItem = FindWidgetItem(client);
    if (newItem != nullptr) {
      ui->client_list_->removeItemWidget(newItem);
      delete newItem;
    }
}

QListWidgetItem* ClientSelector::FindWidgetItem(TcpClientImpl* client) {
    int count = ui->client_list_->count();
    for (int i = 0; i < count; i++) {
      QListWidgetItem *item = ui->client_list_->item(i);
      qint64 tmp_client = item->data(Qt::UserRole + 1).toLongLong();
      if (tmp_client == (qint64)client) {
        return item;
      }
    }
    return nullptr;
}

void ClientSelector::closeEvent(QCloseEvent* e) {
    e->ignore();
    hide();
}

void ClientSelector::on_client_list__itemDoubleClicked(QListWidgetItem *item)
{
    qint64 tmp_client = item->data(Qt::UserRole + 1).toLongLong();
    emit clientSelected((TcpClientImpl*)tmp_client);
}

