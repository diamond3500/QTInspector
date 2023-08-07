#pragma once
#include <QTimer>
#include <QVector>
#include <QList>
#include <QQuickWindow>
#include "qt_window_node.h"
#include "pb/app_window.pb.h"
#include "network/tcp_client_impl.h"
class QtInspectorManagerImpl : public QObject, public TcpClientDelegate {
  Q_OBJECT
public:
  static QtInspectorManagerImpl& Instance();
  void Init(const QString& ip, int port);
  void Uninit();

private slots:
  void ConnectServerTimeout();

private:
  void FindAppWindows(pb::AppInfoRsp& app_info);
  void FindTopWindows(pb::AppInfoRsp& app_info);
 
  void OnConnected(TcpClientImpl* client) override;
  void OnDisConnected(TcpClientImpl* client,
                              QAbstractSocket::SocketError error,
                      const QString& message) override;
  void OnPacketReceived(TcpClientImpl* client,
                        int16_t packet_type,
                        int packet_serial,
                        std::string&& body) override;

  bool eventFilter(QObject* receiver, QEvent* event) override;
  QtWindowNode* FindWindowNode(int window_unique_id);

 private:
  void SendAppInfo(int packet_serial);
  void HandleRefreshWindow(int packet_serial, std::string&& body);
  void HandleSetProperty(int packet_serial, std::string&& body);
  void HandleGetUriContent(int packet_serial, std::string&& body);
  void HandleGetChildDir(int packet_serial, std::string&& body);
  
  void HandleChildAdded(QEvent* event);
  void HandleChildRemoved(QEvent* event);

private:
  QTimer connect_timer_;
  TcpClientImpl* tcp_client_ = nullptr;
  QSet<QObject*> valid_objects;
  QList<QtWindowNode*> window_object_;
  QString ip_;
  int port_ = 0;
};

