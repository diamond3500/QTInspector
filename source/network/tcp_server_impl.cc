#include "tcp_server_impl.h"
#include <qurl.h>
#define MAX_READ_BUFFER 1024

TcpServerImplDelegate::~TcpServerImplDelegate() {}

void TcpServerImpl::OnNewConnection() {
  delegate_->OnNewConnection(new TcpClientImpl(std::unique_ptr<QTcpSocket>(server_->nextPendingConnection())));
}


void TcpServerImpl::StartListen() {
  server_ = std::make_unique<QTcpServer>();
  connect(server_.get(), &QTcpServer::newConnection, this,
          &TcpServerImpl::OnNewConnection);
  connect(server_.get(), &QTcpServer::acceptError, this,
          &TcpServerImpl::OnAcceptError);
  server_->listen(QHostAddress::Any, request_->port);
}

void TcpServerImpl::Shutdown() {
  if (server_) {
    server_->close();
    server_.reset();
  }
}

void TcpServerImpl::OnAcceptError(QAbstractSocket::SocketError error) {
  delegate_->OnShutdown(error, server_->errorString());
}