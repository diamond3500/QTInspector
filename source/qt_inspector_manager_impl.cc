#include "qt_inspector_manager_impl.h"
#include <QGuiApplication>
#include <QQuickView>
#include <QQuickItem>
#include <QQuickWindow>
#include <QApplication>
#include <QWidget>
#include <QBuffer>
#include "network/packet_util.h"
#include "util/qobject_helper.h"

QtInspectorManagerImpl& QtInspectorManagerImpl::Instance()
{
  static QtInspectorManagerImpl s_instance;
  return s_instance;
}

void QtInspectorManagerImpl::Init(const QString& ip, int port) {
  ip_ = ip;
  port_ = port;
  QCoreApplication::instance()->installEventFilter(this);
  // startup timer to get
  connect(&connect_timer_, &QTimer::timeout, this, &QtInspectorManagerImpl::ConnectServerTimeout);
  connect_timer_.start(1000);
}

void QtInspectorManagerImpl::Uninit() {
  qDeleteAll(window_object_);
  window_object_.clear();
  delete tcp_client_;
  tcp_client_ = nullptr;
}

void QtInspectorManagerImpl::SendAppInfo(int packet_serial) {
  qDeleteAll(window_object_);
  window_object_.clear();
  pb::AppInfoRsp app_info;
  app_info.set_appname(QCoreApplication::instance()->applicationName().toStdString());
  FindAppWindows(app_info);
  FindTopWindows(app_info);
  if (0 == packet_serial) {       
    tcp_client_->SendPacket(pb::PacketTypeAppInfoRsp, QObjectHelper::SerialPb(app_info));
  } else {
    tcp_client_->ResponsePacket(pb::PacketTypeAppInfoRsp, packet_serial, QObjectHelper::SerialPb(app_info));
  }
}


void QtInspectorManagerImpl::FindAppWindows(pb::AppInfoRsp& app_info) {
  if (auto guiApp = qobject_cast<QGuiApplication*>(QCoreApplication::instance())) {
    auto windows = guiApp->allWindows();
    for (auto window : windows) {
      auto quick_window = qobject_cast<QQuickWindow*>(window);
      if (!quick_window) {
        continue;
      }
      pb::WindowInfo* info = app_info.add_windows();
      info->set_screenwidth(window->width());
      info->set_screenheight(window->height());
      QtWindowNode* window_object = new QtWindowNode(window, QObjectHelper::GenerateUniqueId());
      {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << *window_object->root_node();
        info->set_objectbuffer(data.data(), data.size());
      }

      {
        QByteArray data;
        QPixmap image = window_object->GrabWindow();
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << image;

        info->set_screenimage(data.data(), data.size());
      }
      window_object_.push_back(window_object);
    }
  }
}

void QtInspectorManagerImpl::FindTopWindows(pb::AppInfoRsp& app_info) {
  QWidgetList widget_list = QApplication::topLevelWidgets();
  for (auto widget : widget_list) {
    pb::WindowInfo* info = app_info.add_windows();
    info->set_screenwidth(widget->width());
    info->set_screenheight(widget->height());
    QtWindowNode* window_object = new QtWindowNode(widget, QObjectHelper::GenerateUniqueId());
    {
      QByteArray data;
      QDataStream stream(&data, QIODevice::WriteOnly);
      stream << *window_object->root_node();
      info->set_objectbuffer(data.data(), data.size());
    }

    {
      QByteArray data;
      QPixmap image = window_object->GrabWindow();
      QDataStream stream(&data, QIODevice::WriteOnly);
      stream << image;

      info->set_screenimage(data.data(), data.size());
    }

    window_object_.push_back(window_object);
  }
}

void QtInspectorManagerImpl::OnConnected(TcpClientImpl* client) {
  SendAppInfo(0);
}

void QtInspectorManagerImpl::OnDisConnected(TcpClientImpl* client,
                                            QAbstractSocket::SocketError error,
                                            const QString& message) {
  qDebug() << "tcp disconnected";
  tcp_client_->deleteLater();
  tcp_client_ = nullptr;
}

void QtInspectorManagerImpl::OnPacketReceived(TcpClientImpl* client,
                                              int16_t packet_type,
                                              int packet_serial,
                                              std::string&& body) {
  BEGIN_HANDLE_PACKET(packet_type)
    HANDLE_PACKET(pb::PacketTypeAppInfoReq, HandleRefreshWindow)
    HANDLE_PACKET(pb::PacketTypeSetPropertyReq, HandleSetProperty)
    HANDLE_PACKET(pb::PacketTypeGetUriContentReq, HandleGetUriContent)
    HANDLE_PACKET(pb::PacketTypeGetChildDirRep, HandleGetChildDir)
  END_HANDLE_PACKET()
}

bool QtInspectorManagerImpl::eventFilter(QObject* receiver, QEvent* event) {
  auto event_type = event->type();
  switch (event_type) {
    case QEvent::ChildAdded:
      HandleChildAdded(event);
      break;
    case QEvent::ChildRemoved:
      HandleChildRemoved(event);
      break;
    default:
      break;
  }

  return QObject::eventFilter(receiver, event);
}

QtWindowNode* QtInspectorManagerImpl::FindWindowNode(int window_unique_id) {
  for (auto window : window_object_) {
    if (window_unique_id == window->window_unique_id()) {
      return window;
    }
  }
  return nullptr;
}


void QtInspectorManagerImpl::ConnectServerTimeout() {
  if (!tcp_client_) {
    auto request = std::make_unique<TcpClientImpl::Request>();
    request->port = port_;
    request->server = ip_;
    tcp_client_ = new TcpClientImpl(std::move(request), this);
    tcp_client_->Connect();
  }
}

void QtInspectorManagerImpl::HandleRefreshWindow(int packet_serial, std::string&& body) {
  SendAppInfo(packet_serial);
}

void QtInspectorManagerImpl::HandleSetProperty(int packet_serial, std::string&& body) {
  pb::SetPropertyReq command;
  command.ParseFromString(body);
  auto window_object = FindWindowNode(command.windowuniqueid());
  if (!window_object) {
    return;
  }
  auto object_node = window_object->FindQObjectByUniqueId(command.propertyuniqueid());
  if (!object_node) {
    return;
  }
  
  QByteArray buffer(command.value().data(), (int)command.value().size());
  QDataStream data_stream(buffer);
  QVariant value;
  data_stream >> value;
  object_node->SetProperty(QString::fromStdString(command.propertyname()), value);
}

void QtInspectorManagerImpl::HandleGetUriContent(int packet_serial, std::string&& body) {
  pb::GetUriContentReq request;
  request.ParseFromString(body);
  QByteArray buffer = QObjectHelper::ReadUriContent(QString::fromStdString(request.uri()));
  pb::GetUriContentRsp response;
  response.set_uri(request.uri());
  response.set_content(buffer.data(), buffer.size());
  
  tcp_client_->ResponsePacket(pb::PacketTypeGetUriContentRsp, packet_serial, QObjectHelper::SerialPb(response));
}

void QtInspectorManagerImpl::HandleGetChildDir(int packet_serial, std::string&& body) {
  pb::GetChildDirRep request;
  request.ParseFromString(body);

  pb::GetChildDirRsp response;
  response.set_parentdir(request.uri());
  QObjectHelper::EnumDir(QString::fromStdString(request.uri()),
      [&response](QObjectHelper::FileInfo&& file_info, bool& continue_enum) {
        auto info = response.add_info();
        info->set_isfile(file_info.is_file);
        info->set_filesize(file_info.file_size);
        info->set_filepath(file_info.file_path.toStdString());
  });
  
  tcp_client_->ResponsePacket(pb::PacketTypeGetChildDirRsp, packet_serial, QObjectHelper::SerialPb(response));
}


void QtInspectorManagerImpl::HandleChildAdded(QEvent* event) {
  QChildEvent* childEvent = static_cast<QChildEvent*>(event);
  QObject* obj = childEvent->child();
  valid_objects.insert(obj);
}

void QtInspectorManagerImpl::HandleChildRemoved(QEvent* event) {
  QChildEvent* childEvent = static_cast<QChildEvent*>(event);
  QObject* obj = childEvent->child();
  valid_objects.remove(obj);
}
