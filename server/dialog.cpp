#include "dialog.h"
#include "qt_window_node.h"
#include "network/packet_util.h"
#include "resource_manager_dlg.h"
#include "util/qobject_helper.h"
#include "./ui_dialog.h"
#include "pb/app_window.pb.h"
#include "object_property_widget.h"

//background-color: rgba(135, 206, 235, 0.2);
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    auto* object_property_widget = new ObjectPropertyWidget(ui->tabWidget);
    ui->tabWidget->addTab(object_property_widget, QString::fromStdWString(L"属性"));
    connect(this, &Dialog::showProperty, object_property_widget, &ObjectPropertyWidget::OnShowProperty);
    connect(object_property_widget, &ObjectPropertyWidget::sendPacket, this, &Dialog::SendPacket);

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
      setWindowTitle("QTInspector");
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
    setWindowTitle("QTInspector(Disconnect)");
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
  if (current_select_node_) {
    const MetaObjectDetail* detail = current_select_node_->object_detail();
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
    object_proxy_model_ = new ObjectSearchProxyModel();
    object_proxy_model_->setSourceModel(new ObjectNodeModel(std::move(root_node)));
    ui->object_tree_view_->setModel(object_proxy_model_);
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
                        const std::string& body,
                        TcpClientImpl::SendPacketDelegate delegate) {
  if (!current_client_) {
    return;
  }
  current_client_->SendPacket(type, body, std::move(delegate));
}

void Dialog::OnClickNode(const QModelIndex& proxy_index) {
  QModelIndex index = object_proxy_model_->mapToSource(proxy_index);
  current_select_node_ = static_cast<QtObjectNode*>(index.internalPointer());
  const MetaObjectDetail* detail = current_select_node_->object_detail();
  if (!detail) {
    return;
  }
  ShowDialogImageIfNeed();
  ShowMask(*detail);
  emit showProperty(*detail, current_select_node_);
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

void Dialog::UpdateClientSelector(TcpClientImpl* client,
                                  const std::string& body) {
  pb::AppInfoRsp window_info;
  window_info.ParseFromString(body);
  client_selector_->UpdateClient(client, window_info);
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
 auto iter = root_window_info_.find(current_select_node_);
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
  setWindowTitle("QTInspector");
  on_refresh_clicked();
}

void Dialog::on_show_clients_clicked()
{
  client_selector_->raise();
  client_selector_->show();
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

