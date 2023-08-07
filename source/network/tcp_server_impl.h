#pragma once
#include <QTcpServer>
#include "network/tcp_client_impl.h"

class TcpServerImplDelegate {
 public:
  virtual ~TcpServerImplDelegate();
  virtual void OnNewConnection(TcpClientImpl* client) = 0;
  virtual void OnShutdown(QAbstractSocket::SocketError error, const QString& message) = 0;
};

class TcpServerImpl : public QObject {
  Q_OBJECT
 public:
   struct Request {
    uint16_t port;
  };

   TcpServerImpl(std::unique_ptr<TcpServerImpl::Request> request,
                   TcpServerImplDelegate* delegate): request_(std::move(request)), delegate_(delegate) {
  }

  ~TcpServerImpl() { 
    Shutdown();
  }

  void StartListen();

  void Shutdown();

private slots:
  void OnNewConnection();
  void OnAcceptError(QAbstractSocket::SocketError error);

 private:
  std::unique_ptr<TcpServerImpl::Request> request_;
  TcpServerImplDelegate* delegate_;
  std::unique_ptr<QTcpServer> server_;
};
