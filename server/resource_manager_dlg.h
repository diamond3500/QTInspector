#ifndef RESOURCE_MANAGER_DLG_H
#define RESOURCE_MANAGER_DLG_H

#include <QDialog>
#include "network/tcp_server_impl.h"
#include "pb/app_window.pb.h"
#include "resource_node_model.h"
#include "show_uri_content.h"

namespace Ui {
class ResourceManagerDialog;
}

class ResourceManagerDialog : public QDialog, public std::enable_shared_from_this<ResourceManagerDialog> {
  Q_OBJECT

 public:
  explicit ResourceManagerDialog(TcpClientImpl* client,
                                 QWidget* parent = nullptr);
  ~ResourceManagerDialog();

 public slots:
  void OnRemoveClient(TcpClientImpl* client);

 Q_SIGNALS:
  void onGetChildInfo(const QModelIndex& index,
                      const pb::GetChildDirRsp& rsp);

  void onGetContent(const QModelIndex& index,
                    const pb::GetUriContentRsp& rsp);
  

 private slots:
  void on_resource_dir_clicked(const QModelIndex& index);

  void on_resource_dir_customContextMenuRequested(const QPoint &pos);

  void OnSave();
  void OnCopyToClipboard();

private:
  pb::GetUriContentRsp SyncGetContent(const std::string& uri);
 pb::GetChildDirRsp SyncGetChidlDir(const std::string& uri);

private:
  void HandleGetChildDir(const std::string& body, const QModelIndex& index);
 void HandleGetUriContent(const std::string& body, const QModelIndex& index);

  void GetChildDir(ResourceNode* node, const QModelIndex& index);
  void GetContent(ResourceNode* node, const QModelIndex& index);

  void SaveFile(const ResourceNode* node);
  void SaveDir(const ResourceNode* node);
  void EnsureSaveDir(const std::string& uri, const QString& save_target_dir);


 private:
  Ui::ResourceManagerDialog* ui;
  TcpClientImpl* client_;
  ShowUriContent* show_content_ = nullptr;
  //menu
  QAction save_action_;
  QAction copy_action_;
};

#endif  // RESOURCE_MANAGER_DLG_H
