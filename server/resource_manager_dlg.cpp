#include "resource_manager_dlg.h"
#include "ui_resource_manager_dlg.h"
#include  "util/qobject_helper.h"
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QTimer>

#define CHECK_CLIENT() \
  if (client_ == nullptr) { \
    return; \
  }

ResourceManagerDialog::ResourceManagerDialog(TcpClientImpl* client, QWidget* parent) :
    QDialog(parent), ui(new Ui::ResourceManagerDialog), 
    client_(client) {
    ui->setupUi(this);
  show_content_ = new ShowUriContent(this);
    ui->splitter->addWidget(show_content_);
  auto model = new ResourceNodeModel(client);
  connect(this, &ResourceManagerDialog::onGetChildInfo, model, &ResourceNodeModel::OnGetChildInfo);
  connect(this, &ResourceManagerDialog::onGetContent, model, &ResourceNodeModel::OnGetContent);
  model->SetHeader(QStringList() << "Name" << "Size");
  ui->resource_dir->setModel(model);
  ui->resource_dir->setContextMenuPolicy(Qt::CustomContextMenu);
  setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

  //menu
  save_action_.setText("save");
  connect(&save_action_, &QAction::triggered, this, &ResourceManagerDialog::OnSave);

  copy_action_.setText("copy to clipboard"); 
  connect(&copy_action_, &QAction::triggered, this, &ResourceManagerDialog::OnCopyToClipboard);
}

ResourceManagerDialog::~ResourceManagerDialog()
{
    delete ui;
}

void ResourceManagerDialog::OnRemoveClient(TcpClientImpl* client) {
  if (client_ == client) {
    client_ = nullptr;
  }
}

void ResourceManagerDialog::HandleGetChildDir(const std::string& body, const QModelIndex &index) {
  pb::GetChildDirRsp rsp;
  rsp.ParseFromString(body);
  emit onGetChildInfo(index, rsp);
}

void ResourceManagerDialog::HandleGetUriContent(const std::string& body, const QModelIndex &index) {
  pb::GetUriContentRsp rsp;
  rsp.ParseFromString(body);
  emit onGetContent(index, rsp);
  show_content_->SetContent(rsp);
}

void ResourceManagerDialog::GetChildDir(ResourceNode* node, const QModelIndex &index) {
  CHECK_CLIENT();
  pb::GetChildDirRep request;
  request.set_resure(false);
  request.set_uri(node->path_.toStdString());
  auto weak_self = weak_from_this();
  ui->resource_dir->setEnabled(false);
  client_->SendPacket(pb::PacketTypeGetChildDirRep, QObjectHelper::SerialPb(request), [weak_self, this, index](std::string&& body) {
      if (auto self = weak_self.lock()) {
        HandleGetChildDir(std::move(body), index);
        ui->resource_dir->expand(index);
        ui->resource_dir->setEnabled(true);
      }
  });
}

void ResourceManagerDialog::GetContent(ResourceNode* node, const QModelIndex &index) {
  CHECK_CLIENT();
  pb::GetUriContentReq request;
  request.set_uri(node->path_.toStdString());
  auto weak_self = weak_from_this();
  ui->resource_dir->setEnabled(false);
  client_->SendPacket(pb::PacketTypeGetUriContentReq, QObjectHelper::SerialPb(request), [weak_self, this, index](std::string&& body) {
      if (auto self = weak_self.lock()) {
         HandleGetUriContent(std::move(body), index);
        ui->resource_dir->setEnabled(true);
      }
  });
}

void ResourceManagerDialog::SaveFile(const ResourceNode* node) {
  const QString file_path = QFileDialog::getSaveFileName(this);
  if (file_path.isEmpty()) {
    return;
  }
  auto& content = node->response.content();
  if (content.empty()) {
    pb::GetUriContentRsp rsp = SyncGetContent(node->path_.toStdString());
    QObjectHelper::SaveFile(file_path, rsp.content().data(), (int)rsp.content().length());
  } else {
    QObjectHelper::SaveFile(file_path, content.data(), (int)content.length());
  }
}

void ResourceManagerDialog::SaveDir(const ResourceNode* node) {
  QString target_dir = QFileDialog::getExistingDirectory(this, tr("Save As"));
  if (target_dir.isEmpty()) {
    return;
  }
  setEnabled(false);
  QTimer progress_timer(this);
  int progress = 10;
  connect(&progress_timer, &QTimer::timeout, [this, &progress]() {
    ui->save_progress->setValue(progress);
    progress += 10;
    if (progress == 100) {
      progress = 10;
    }
  });

  progress_timer.start(500);
  ui->save_progress->show();
  EnsureSaveDir(node->path_.toStdString(), target_dir);
  ui->save_progress->hide();
  setEnabled(true);
}

void ResourceManagerDialog::EnsureSaveDir(const std::string& uri, const QString& save_target_dir) {
  CHECK_CLIENT();
  QObjectHelper::CreateDirectoryX(save_target_dir);
  pb::GetChildDirRsp rsp = SyncGetChidlDir(uri);
  for (auto& child : rsp.info()) {
    QString dest_path = QObjectHelper::MakePath(save_target_dir, QObjectHelper::GetFileName(QString::fromStdString(child.filepath())));
    if (child.isfile()) {
      pb::GetUriContentRsp content = SyncGetContent(child.filepath());     
      QObjectHelper::SaveFile(dest_path, content.content().data(), (int)content.content().length());
    } else {
      EnsureSaveDir(child.filepath(), dest_path);
    }
  }
}

void ResourceManagerDialog::on_resource_dir_clicked(const QModelIndex &index) {
  auto current_select_node = static_cast<ResourceNode*>(index.internalPointer());
  if (current_select_node->hasResponse) {
    show_content_->SetContent(current_select_node->response);
    return;
  }
  CHECK_CLIENT();
  if (current_select_node->is_file) {
    GetContent(current_select_node, index);
  } else {
    GetChildDir(current_select_node, index);
  }
}


void ResourceManagerDialog::on_resource_dir_customContextMenuRequested(const QPoint &pos)
{
  CHECK_CLIENT();
  QMenu menu;
  menu.addAction(&save_action_);

  QModelIndex index = ui->resource_dir->currentIndex();
  auto node = static_cast<ResourceNode*>(index.internalPointer());
  if (node->is_file) {
    menu.addAction(&copy_action_);
  }
  
  menu.exec(QCursor::pos());
}

void ResourceManagerDialog::OnSave() {
  QModelIndex index = ui->resource_dir->currentIndex();
  auto node = static_cast<ResourceNode*>(index.internalPointer());
  if (node->is_file) {
    SaveFile(node);
  } else {
    SaveDir(node);
  }
}

void ResourceManagerDialog::OnCopyToClipboard() {
  QModelIndex index = ui->resource_dir->currentIndex();
  auto node = static_cast<ResourceNode*>(index.internalPointer());
  if (!node->hasResponse) {
    node->response = SyncGetContent(node->path_.toStdString());
    node->hasResponse = true;
  }
  QClipboard* clipboard = QApplication::clipboard();
  if (node->path_.endsWith(".png")) {
    QPixmap pixmap;
    pixmap.loadFromData((const uchar*)node->response.content().data(),
                        (uint)node->response.content().length());
    clipboard->setPixmap(pixmap);
  } else {
    clipboard->setText(QString::fromStdString(node->response.content()));
  }
}

pb::GetUriContentRsp ResourceManagerDialog::SyncGetContent(const std::string& uri) {
  QEventLoop loop;
  pb::GetUriContentReq request;
  request.set_uri(uri);   
  pb::GetUriContentRsp rsp;
  client_->SendPacket(pb::PacketTypeGetUriContentReq, QObjectHelper::SerialPb(request), [&loop, &rsp, this](std::string&& body) {  
    rsp.ParseFromString(body); 
    loop.quit();
  });
  loop.exec();
  return rsp;
}

pb::GetChildDirRsp ResourceManagerDialog::SyncGetChidlDir(const std::string& uri) {
  QEventLoop loop;
  pb::GetChildDirRep request;
  request.set_resure(false);
  request.set_uri(uri);
  pb::GetChildDirRsp rsp;
  client_->SendPacket(pb::PacketTypeGetChildDirRep, QObjectHelper::SerialPb(request), [&loop, &rsp, this](std::string&& body) {
    rsp.ParseFromString(body);
    loop.quit();
  });
  loop.exec();
  return rsp;
}

