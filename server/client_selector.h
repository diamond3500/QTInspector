#ifndef CLIENT_SELECTOR_H
#define CLIENT_SELECTOR_H

#include <QWidget>
#include <QListWidgetItem>
#include <QCloseEvent>
#include "network/tcp_server_impl.h"
#include "pb/app_window.pb.h"
namespace Ui {
class ClientSelector;
}

class ClientSelector : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void clientSelected(TcpClientImpl* client);

public:
    explicit ClientSelector(QWidget *parent = nullptr);
    ~ClientSelector();

    void UpdateClient(TcpClientImpl* client, const pb::AppInfoRsp& app_info);
    
public slots:
    void OnRemoveClient(TcpClientImpl* client);

private slots:
    void on_client_list__itemDoubleClicked(QListWidgetItem *item);

private:
 QListWidgetItem* FindWidgetItem(TcpClientImpl* client);
 void closeEvent(QCloseEvent* e) override;

private:
    Ui::ClientSelector *ui;
};

#endif // CLIENT_SELECTOR_H
