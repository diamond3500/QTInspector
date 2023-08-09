#include "dialog.h"
#include "qt_window_node.h"
#include "network/packet_util.h"
#include <QColorDialog>
#include <QFontDialog>
#include <QCheckBox>
#include <QDesktopServices>
#include "resource_manager_dlg.h"
#include "util/qobject_helper.h"
#include "./ui_dialog.h"

#define PROPERTY_NAME "custom_key"
#define PROPERTY_VALUE "custom_value"

#define PROPERTY_NAME_ROLE (Qt::UserRole + 1)
#define PROPERTY_VALUE_ROLE (Qt::UserRole + 2)

#define BEGIN_HANDLE_PROPERTY_VALUE(type_id) switch (type_id) {
#define HANDLE_PROPERTY_VALUE(t, fun) \
  case t: {                           \
    fun(property_name, row, value);   \
  } break;

#define HANDLE_DEFAULT_PROPERTY_VALUE(fun)\
  default: \
  fun(property_name, row, value); \
  break; \

#define END_HANDLE_PROPERTY_VALUE() }


//background-color: rgba(135, 206, 235, 0.2);
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

  auto request = std::make_unique<TcpServerImpl::Request>();
  request->port = 5973;
  tcp_server_ = QSharedPointer<TcpServerImpl>::create(std::move(request), this);
  tcp_server_->StartListen();

  connect(ui->object_tree_view_, &QAbstractItemView::clicked, this, &Dialog::OnClickNode);
  cover_mask_ = std::make_unique<FloatCoverMask>(this);
  cover_mask_->setVisible(false);

  client_selector_ = std::make_unique <ClientSelector>();
  connect(this, &Dialog::removeClient, client_selector_.get(), &ClientSelector::OnRemoveClient);

  show_uri_content = std::make_unique<ShowUriContentDlg>();

  connect(client_selector_.get(), &ClientSelector::clientSelected, this, &Dialog::OnClientSelected);
  
  connect(ui->object_detail_view_, &QTableWidget::cellChanged, this,
          &Dialog::OnPropertyValueEdit);
}

#define CHECK_CLIENT() \
  if (!current_client_) return


Dialog::~Dialog()
{
  qDeleteAll(clients_);
  delete ui;
}

void Dialog::OnNewConnection(TcpClientImpl* client) {
  client->SetDelegate(this);
  clients_.push_back(client);
  if (nullptr == current_client_) {
    current_client_ = client;
    EnableUI();
  }
}

void Dialog::OnShutdown(QAbstractSocket::SocketError error,
                        const QString& message) {}

void Dialog::OnConnected(TcpClientImpl* client) {}

void Dialog::OnDisConnected(TcpClientImpl* client,
                            QAbstractSocket::SocketError error,
                            const QString& message) {
  emit removeClient(client);
  if (current_client_ == client) {
    current_client_ = nullptr;
  }
  clients_.removeAll(client);
  client->deleteLater();
}

void Dialog::OnPacketReceived(TcpClientImpl* client,
                              int16_t packet_type,
                              int packet_serial,
                              std::string&& body) {
  emit receivedPacket(client, packet_type, packet_serial, body);
  if (pb::PacketTypeAppInfoRsp == packet_type) {
    UpdateClientSelector(client, body);
  }
  if (client == current_client_) {
    HandlePacket(packet_type, packet_serial, std::move(body));
  }
}

void Dialog::closeEvent(QCloseEvent* e) {
  client_selector_.reset();
  e->accept();
}

void Dialog::resizeEvent(QResizeEvent*) {
  if (current_select_node) {
    const MetaObjectDetail* detail = current_select_node->object_detail();
    if (!detail) {
      return;
    }
    ShowMask(*detail);
  }
}

void Dialog::HandlePacket(int16_t packet_type, int packet_serial, std::string&& body) {
  BEGIN_HANDLE_PACKET(packet_type)
    HANDLE_PACKET(pb::PacketTypeAppInfoRsp, HandleWindowInfo)
    HANDLE_PACKET(pb::PacketTypeGetUriContentRsp, HandleUriContent)
  END_HANDLE_PACKET()
}

void Dialog::HandleWindowInfo(int packet_serial, std::string&& body) {
  if (body.empty()) {
    return;
  }
  root_window_info_.clear();
  pb::AppInfoRsp app_info;
  app_info.ParseFromString(body);
  {
    auto root_node = std::make_unique<QtObjectNode>(nullptr);
    for (auto& window_info : app_info.windows()) {
      // set object model
      QByteArray object_byte_array(window_info.objectbuffer().data(),
                                   (int)window_info.objectbuffer().size());
      QDataStream data_stream(object_byte_array);
      auto object_node = new QtObjectNode(root_node.get());
      data_stream >> *object_node;
      root_node->AddChild(object_node);
      root_window_info_.insert(object_node, ConvertWindowInfo(window_info));
    }
    object_proxy_model_ = std::make_unique<ObjectSearchProxyModel>();
    object_proxy_model_->setSourceModel(new ObjectNodeModel(std::move(root_node)));
    ui->object_tree_view_->setModel(object_proxy_model_.get());
  }
}

void Dialog::HandleUriContent(int packet_serial, std::string&& body) {
  pb::GetUriContentRsp response;
  response.ParseFromString(body);

  show_uri_content->SetContent(response);
  show_uri_content->show();
  show_uri_content->raise();
}

void Dialog::ShowMask(const MetaObjectDetail& detail) {
   QPointF maped_screen_pt = ui->screen_image->mapTo(this, QPoint(0, 0));
  double screen_x = detail.PropertyValueForKey("screen_x").toDouble() * screen_image_scale_ + maped_screen_pt.x();
  double screen_y = detail.PropertyValueForKey("screen_y").toDouble() * screen_image_scale_ + maped_screen_pt.y();
  double width = detail.PropertyValueForKey("width").toDouble() * screen_image_scale_;
  double height = detail.PropertyValueForKey("height").toDouble() * screen_image_scale_;
  cover_mask_->setGeometry(static_cast<int>(screen_x), static_cast<int>(screen_y), 
    static_cast<int>(width), static_cast<int>(height));
  cover_mask_->show();
}

void Dialog::SendPacket(pb::PacketType type,
                        std::string&& body,
                        TcpClientImpl::SendPacketDelegate delegate) {
  if (!current_client_) {
    return;
  }
  current_client_->SendPacket(type, std::move(body), std::move(delegate));
}

void Dialog::OnClickNode(const QModelIndex& proxy_index) {
  QModelIndex index = object_proxy_model_->mapToSource(proxy_index);
  current_select_node = static_cast<QtObjectNode*>(index.internalPointer());
  const MetaObjectDetail* detail = current_select_node->object_detail();
  if (!detail) {
    return;
  }
  ShowDialogImageIfNeed();
  ShowMask(*detail);
  InitPropertyTable(*detail);
}
void Dialog::on_refresh_clicked()
{
  CHECK_CLIENT();
  ui->refresh->setEnabled(false);
  SendPacket(pb::PacketTypeAppInfoReq, "", [this](std::string&& body) {
    UpdateClientSelector(current_client_, body);
    HandleWindowInfo(0, std::move(body));
    ui->refresh->setEnabled(true);
  });
}

void Dialog::InnerSetProperty(
    const QString& property_name,
    const QVariant& value,
    TcpClientImpl::SendPacketDelegate delegate) {
  if (nullptr == current_select_node) {
    return;
  }
  pb::SetPropertyReq command;
  command.set_propertyname(property_name.toStdString());
  command.set_windowuniqueid(current_select_node->window_unique_id());
  command.set_propertyuniqueid(current_select_node->unique_id());
 
  QByteArray body;
  QDataStream stream(&body, QIODevice::WriteOnly);
  stream << value;
  command.set_value(body.data(), body.size());

  std::string serial_data;
  command.SerializeToString(&serial_data);
  SendPacket(pb::PacketTypeSetPropertyReq, std::move(serial_data), std::move(delegate));
}

void Dialog::UpdateClientSelector(TcpClientImpl* client,
                                  const std::string& body) {
  pb::AppInfoRsp window_info;
  window_info.ParseFromString(body);
  client_selector_->UpdateClient(client, window_info);
}


void Dialog::ShowPropertyValue(const QString& property_name,
                               int row,
                               const QVariant& value) {
  int type_id = value.type();
  BEGIN_HANDLE_PROPERTY_VALUE(type_id)
    HANDLE_PROPERTY_VALUE(QMetaType::Bool, HandleBoolProperty)
    HANDLE_PROPERTY_VALUE(QMetaType::QColor, HandleColorProperty)
    HANDLE_PROPERTY_VALUE(QMetaType::QFont, HandleFontProperty)   
    HANDLE_PROPERTY_VALUE(QMetaType::QUrl, HandleUrlProperty)
    HANDLE_DEFAULT_PROPERTY_VALUE(HandleDefaultProperty);
  END_HANDLE_PROPERTY_VALUE()
}

void Dialog::HandleBoolProperty(const QString& property_name,
                                int row,
                                const QVariant& value) {
  QCheckBox* checkbox = new QCheckBox(ui->object_detail_view_);
  checkbox->setChecked(value.toBool());
  checkbox->setProperty(PROPERTY_NAME, property_name);
  checkbox->setProperty(PROPERTY_VALUE, value);
  connect(checkbox, &QCheckBox::stateChanged, this, &Dialog::OnProperyValueStateChange);
  ui->object_detail_view_->setCellWidget(row, 1, checkbox);
}

void Dialog::HandleColorProperty(const QString& property_name,
                                 int row,
                                 const QVariant& value) {
  QPushButton* btn = new QPushButton(ui->object_detail_view_);
  QPalette palette = btn->palette();
  palette.setColor(QPalette::ButtonText, value.value<QColor>());
  btn->setPalette(palette);
  btn->setText(value.toString());
  ui->object_detail_view_->setCellWidget(row, 1, btn);
  btn->setProperty(PROPERTY_NAME, property_name);
  btn->setProperty(PROPERTY_VALUE, value);
  connect(btn, &QAbstractButton::clicked, this, &Dialog::OnClickChangeColor);
}

void Dialog::HandleFontProperty(const QString& property_name,
                                int row,
                                const QVariant& value) {
  QPushButton* btn = new QPushButton(ui->object_detail_view_);
  btn->setText(value.toString());
  ui->object_detail_view_->setCellWidget(row, 1, btn);
  btn->setProperty(PROPERTY_NAME, property_name);
  btn->setProperty(PROPERTY_VALUE, value);
  connect(btn, &QAbstractButton::clicked, this, &Dialog::OnClickChangeFont);
}

void Dialog::HandleUrlProperty(const QString& property_name,
                               int row,
                               const QVariant& value) {
  QWidget* widget = new QWidget(this);
  QHBoxLayout* hbox = new QHBoxLayout();
  QLabel* text = new QLabel(widget);
  text->setText(value.toString());
  hbox->addWidget(text);
  QPushButton* btn = new QPushButton(widget);
  btn->setText(QString::fromStdWString(L"浏览"));
  btn->setProperty(PROPERTY_NAME, property_name);
  btn->setProperty(PROPERTY_VALUE, value);
  hbox->addWidget(btn);
  hbox->setStretch(0, 1);
  hbox->setContentsMargins(0, 0, 0, 0);
  widget->setLayout(hbox);
  connect(btn, &QAbstractButton::clicked, this, &Dialog::OnClickBrowser);
  ui->object_detail_view_->setCellWidget(row, 1, widget);
}

void Dialog::HandleDefaultProperty(const QString& property_name,
                                   int row,
                                   const QVariant& value) {
  QTableWidgetItem* property_value = new QTableWidgetItem;
  property_value->setText(value.toString());
  property_value->setData(PROPERTY_NAME_ROLE, property_name);
  property_value->setData(PROPERTY_VALUE_ROLE, value);
  ui->object_detail_view_->setItem(row, 1, property_value);
}

void Dialog::EnableUI() {
  ui->refresh->setEnabled(true);
}

DialogInfo Dialog::ConvertWindowInfo(const ::pb::WindowInfo& window_info) {
  DialogInfo dialog_info; 
  auto& screen_image_data = window_info.screenimage();
  QByteArray object_byte_array(screen_image_data.data(),
                               (int)screen_image_data.size());
  QDataStream data_stream(object_byte_array);
  data_stream >> dialog_info.pixmap;
  dialog_info.screen_width = window_info.screenwidth();
  dialog_info.screen_height = window_info.screenheight();
  return dialog_info;
}

void Dialog::ShowDialogImageIfNeed() {
 auto iter = root_window_info_.find(current_select_node);
  if (iter == root_window_info_.end()) {
    return;
  }

  if (!iter->pixmap.isNull()) {
    int image_width = iter->pixmap.width();
    int image_height = iter->pixmap.height();
    const static int MAX_SCREEN_HEIGHT = 480;
    int image_show_height =
        image_height > MAX_SCREEN_HEIGHT ? MAX_SCREEN_HEIGHT : image_height;
    int image_show_width = image_width * image_show_height / image_height;
    ui->screen_image->setFixedWidth(image_show_width);
    ui->screen_image->setFixedHeight(image_show_height);
    ui->screen_image->setPixmap(iter->pixmap);
    screen_image_scale_ =
        static_cast<float>(image_show_width) / iter->screen_width;
  }
}

void Dialog::OnClientSelected(TcpClientImpl* client) {
  current_client_ = client;
  on_refresh_clicked();
}

void Dialog::on_show_clients_clicked()
{
  client_selector_->raise();
  client_selector_->show();
}

void Dialog::OnClickChangeColor() {
  QPushButton* btn = (QPushButton*)sender();
  const QColor color = QColorDialog::getColor(btn->property(PROPERTY_VALUE).value<QColor>());
  if (color.isValid()) {
    InnerSetProperty(btn->property(PROPERTY_NAME).value<QString>(), color);
  }
}

void Dialog::OnClickChangeFont() {
  QPushButton* btn = (QPushButton*)sender();
  bool ok = false;
  const QFont font = QFontDialog::getFont(&ok, btn->property(PROPERTY_VALUE).value<QFont>());
  if (ok) {
    InnerSetProperty(btn->property(PROPERTY_NAME).value<QString>(), font);
  }
}

void Dialog::OnClickBrowser() {
  QPushButton* btn = (QPushButton*)sender();
  QString uri = btn->property(PROPERTY_VALUE).toString();
  if (uri.startsWith("qrc:") || uri.startsWith("file://")) {
    pb::GetUriContentReq request;
    request.set_uri(uri.toStdString());
    SendPacket(pb::PacketTypeGetUriContentReq, QObjectHelper::SerialPb(request), 0);
  }
}

void Dialog::OnProperyValueStateChange(int state) {
  QCheckBox* btn = (QCheckBox*)sender();
  QVariant value(false);
  if (state == Qt::Checked) {
    value = true;
  }
  InnerSetProperty(btn->property(PROPERTY_NAME).value<QString>(), value);
}

void Dialog::OnPropertyValueEdit(int row, int column) {
  QTableWidgetItem* widget_item = ui->object_detail_view_->item(row, column);
  QVariant key = widget_item->data(PROPERTY_NAME_ROLE);
  QVariant value = widget_item->data(PROPERTY_VALUE_ROLE);
  QString text = widget_item->text();
  switch (value.type()) {
    case QMetaType::Double: {
      InnerSetProperty(key.toString(), text.toDouble());
    } break;
    case QMetaType::QString: {
      InnerSetProperty(key.toString(), text);
    } break;
    case QMetaType::Int: {
      InnerSetProperty(key.toString(), text.toInt());
    } break;
    case QMetaType::QUrl: {
      
    } break;
    default: {
      qDebug() << "unsupport value:" << value.type();
    } break;
  }
}

void Dialog::InitPropertyTable(const MetaObjectDetail& detail) {
  QStringList keys = detail.PropertyKeys();
  int row = 0;
  ui->object_detail_view_->blockSignals(true);
  ui->object_detail_view_->clear();
  //table header
  QTableWidgetItem* property_name = new QTableWidgetItem;
  QTableWidgetItem* property_value = new QTableWidgetItem;
  property_name->setText(QString::fromStdWString(L"属性名"));
  property_value->setText(QString::fromStdWString(L"属性值"));
  ui->object_detail_view_->setHorizontalHeaderItem(0, property_name);
  ui->object_detail_view_->setHorizontalHeaderItem(1, property_value);

  ui->object_detail_view_->setRowCount((int)keys.size());
  for (auto& key : keys) {
    property_name = new QTableWidgetItem;
    property_name->setFlags(property_name->flags() & (~Qt::ItemIsEditable));
    property_name->setText(key);
    ui->object_detail_view_->setItem(row, 0, property_name);

    auto& value = detail.PropertyValueForKey(key);
    ShowPropertyValue(key, row, value);
    row++;
  }
  ui->object_detail_view_->blockSignals(false);
}
void Dialog::on_resource_manager_clicked()
{
  if (nullptr == current_client_) {
    return;
  }
  auto dlg = std::make_shared<ResourceManagerDialog>(current_client_);
  connect(this, &Dialog::removeClient, dlg.get(),
          &ResourceManagerDialog::OnRemoveClient);
  dlg->exec();
}


void Dialog::on_search_text_returnPressed()
{
  if (nullptr == object_proxy_model_) {
    return;
  }

  object_proxy_model_->SetSearchText(ui->search_text->text());
  ui->object_tree_view_->expandAll();
}

void Dialog::on_expand_clicked()
{
  ui->object_tree_view_->expandAll();
}

void Dialog::on_collapse_clicked()
{
    ui->object_tree_view_->collapseAll();
}

