#include "tcp_client_impl.h"
#include <qurl.h>
#include "packet_util.h"
TcpClientDelegate::~TcpClientDelegate() {}

TcpClientImpl::TcpClientImpl(std::unique_ptr<QTcpSocket> client)
    : socket_(std::move(client)) {}

TcpClientImpl::~TcpClientImpl() {
  Disconnect();
  for (auto& delegate : send_packet_delegate_map_) {
    delegate("");
  }
}

void TcpClientImpl::Connect() {
  socket_ = std::make_unique<QTcpSocket>(this);
  ConnectSocketSignal();
  QUrl url(request_->server);

  if (url.host().isEmpty()) {
    socket_->connectToHost(request_->server, request_->port);
  } else {
    socket_->connectToHost(url.host(), request_->port);
  }
}

int TcpClientImpl::SendPacket(int16_t ptype,
                               const std::string& data,
                               SendPacketDelegate delegate) {
  std::string head;
  int packet_serial = ++packet_serial_;
  packet_serial = PacketUtil::PacketData(true, ptype, packet_serial, data, head);
  Send(head.data(), static_cast<int>(head.length()));
  Send(data.data(), static_cast<int>(data.length()));
  socket_->flush();
  if (delegate) {
    send_packet_delegate_map_[packet_serial] = std::move(delegate);
  }
  return packet_serial;
}

int TcpClientImpl::ResponsePacket(int16_t ptype,
                                  int packet_serial,
                                  std::string&& data) {
  std::string head;
  packet_serial = PacketUtil::PacketData(false, ptype, packet_serial, data, head);
  Send(head.data(), static_cast<int>(head.length()));
  Send(data.data(), static_cast<int>(data.length()));
  socket_->flush();
  return packet_serial;
}

void TcpClientImpl::OnSocketStateChanged(QAbstractSocket::SocketState state) {
  switch (state)
  {
  case QAbstractSocket::ConnectedState:
    delegate_->OnConnected(this);
    break;
  default:
    break;
  }
}

void TcpClientImpl::OnSocketReadyRead() {
  recv_buffer_.append(socket_->readAll());
  while (true) {
    int16_t packet_type = 0;
    int packet_serial = 0;
    bool is_request = false;
    std::string body;
    if (!PacketUtil::UnPacketData(recv_buffer_, is_request, packet_type, packet_serial, body)) {
      break;
    }
    if (is_request) {
      delegate_->OnPacketReceived(this, packet_type, packet_serial, std::move(body));
    } else {
      auto iter = send_packet_delegate_map_.find(packet_serial);
      if (iter != send_packet_delegate_map_.end()) {
        iter.value()(std::move(body));
        send_packet_delegate_map_.erase(iter);
      } else {
        delegate_->OnPacketReceived(this, packet_type, packet_serial, std::move(body));
      }
    }
  }
}

void TcpClientImpl::OnSocketError(QAbstractSocket::SocketError error) {
  delegate_->OnDisConnected(this, error, socket_->errorString());
}

void TcpClientImpl::ConnectSocketSignal() {
  connect(socket_.get(), &QTcpSocket::stateChanged, this,
          &TcpClientImpl::OnSocketStateChanged);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  connect(socket_.get(), &QTcpSocket::errorOccurred, this, &TcpClientImpl::OnSocketError);
  #else
  connect(socket_.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
          this, &TcpClientImpl::OnSocketError);
#endif

  connect(socket_.get(), &QTcpSocket::readyRead, this,
          &TcpClientImpl::OnSocketReadyRead);
}

void TcpClientImpl::Send(const QByteArray& data) {
  socket_->write(data);
}

void TcpClientImpl::Send(const void* data, int len) {
  socket_->write((const char*)data, len);
}

void TcpClientImpl::Disconnect() {
  socket_.reset();
}

void TcpClientImpl::SetDelegate(TcpClientDelegate* delegate) {
  if (nullptr == delegate_) {
    ConnectSocketSignal();
  }
  delegate_ = delegate;
}
