#include "resource_node_model.h"
#include "pb/app_window.pb.h"
#include  "util/qobject_helper.h"
#include <QIcon>

QMap<QString, QIcon> ResourceNodeModel::type_icon_map_;
ResourceNodeModel::ResourceNodeModel(QObject* parent) {
  root_node_ = std::make_unique<ResourceNode>(":/", nullptr);
  InitTypeIcon();
}

ResourceNode* ResourceNodeModel::FindNode(const QString& path) {
  return FindChildNode(root_node_.get(), path);
}


void ResourceNodeModel::OnGetChildInfo(const QModelIndex& index,
                                       const pb::GetChildDirRsp& rsp) {
  auto parent_node = static_cast<ResourceNode*>(index.internalPointer());
  parent_node->hasResponse = true;
  beginInsertRows(index, 0, 0);
  for (auto& item : rsp.info()) {
    ResourceNode* node = new ResourceNode(QString::fromStdString(item.filepath()), parent_node);
    node->is_file = item.isfile();
    node->file_size_ = item.filesize();
    parent_node->children_.push_back(node);
  }
  endInsertRows();
}

void ResourceNodeModel::OnGetContent(const QModelIndex& index,
                                     const pb::GetUriContentRsp& rsp) {
  auto parent_node = static_cast<ResourceNode*>(index.internalPointer());
  parent_node->hasResponse = true;
  parent_node->response = rsp;
}

QVariant ResourceNodeModel::OnShowData(const QModelIndex& index,
                                       int role,
                                       ResourceNode* node) const {
  switch (role) {
    case Qt::DisplayRole: {
      int column = index.column();
      if (column == 0) {
        return QVariant(QString("%1").arg(QObjectHelper::GetFileName(node->path_)));
      } else if (column == 1 && node->is_file) {
        return QVariant(node->file_size_);
      }
    }
    case Qt::DecorationRole: {
      if (!node->is_file && index.column() == 0) {
        return type_icon_map_["folder"];
      }
    }
    default: {      
    }
  }
  return QVariant();
}

ResourceNode* ResourceNodeModel::FindChildNode(ResourceNode* parent,
                                               const QString& path) {
  if (parent->path_ == path) {
    return parent;
  }
  for (auto child : parent->children_) {
    auto find = FindChildNode(child, path);
    if (find) {
      return find;
    }
  }
  return nullptr;
}

void ResourceNodeModel::InitTypeIcon() {
  if (type_icon_map_.empty()) {
    type_icon_map_["folder"] = QIcon(":/icons/resource/folder.png");
  }
}