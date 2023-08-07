#pragma once
#include <QTcpSocket>

class TcpClientImpl;
class TcpClientDelegate {
 public:
  virtual ~TcpClientDelegate();
  virtual void OnConnected(TcpClientImpl* client) = 0;
  virtual void OnDisConnected(TcpClientImpl* client,
                              QAbstractSocket::SocketError error,
                              const QString& message) = 0;
  virtual void OnPacketReceived(TcpClientImpl* client,
                                int16_t packet_type,
                                int packet_serial,
                                std::string&& body) = 0;
};

class TcpClientImpl : public QObject {
  Q_OBJECT
 public:
   using SendPacketDelegate = std::function<void(std::string&& body)>;
   struct Request {
    QString server;
    uint16_t port;
  };

   TcpClientImpl(std::unique_ptr<TcpClientImpl::Request> request,
                   TcpClientDelegate* delegate): request_(std::move(request)), delegate_(delegate) {
  }

   TcpClientImpl(std::unique_ptr<QTcpSocket> client);

  ~TcpClientImpl();

  void Connect();

  int SendPacket(int16_t ptype,
                  std::string&& data,
                  SendPacketDelegate delegate = nullptr);

  int ResponsePacket(int16_t ptype, int packet_serial, std::string&& data);

  void Disconnect();

  void SetDelegate(TcpClientDelegate* delegate);

private slots:
  void OnSocketStateChanged(QAbstractSocket::SocketState state);
  void OnSocketReadyRead();
  void OnSocketError(QAbstractSocket::SocketError error);

private:
  void Send(const QByteArray& data);
  void Send(const void* data, int len);
  void ConnectSocketSignal();

   std::unique_ptr<TcpClientImpl::Request> request_;
   TcpClientDelegate* delegate_ = nullptr;
   std::unique_ptr<QTcpSocket> socket_;
   QByteArray recv_buffer_;
   QMap<int, SendPacketDelegate> send_packet_delegate_map_;
   int packet_serial_ = 1;
};
