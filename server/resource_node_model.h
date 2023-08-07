#ifndef RESOURCENODEMODEL_H
#define RESOURCENODEMODEL_H

#include <QAbstractItemModel>
#include "tree_view_base_model.h"
#include "pb/app_window.pb.h"

struct ResourceNode {
  ResourceNode(const QString& _path, ResourceNode* parent)
      : path_(_path), parent_(parent) {}
  ~ResourceNode() { qDeleteAll(children_); }

  const QList<ResourceNode*>& children() const { return children_; }

  const ResourceNode* parent() const { return parent_; }

  QString path_;
  pb::GetUriContentRsp response;

  ResourceNode* parent_ = nullptr;
  QList<ResourceNode*> children_;
  int64_t file_size_ = 0;
  bool is_file = false;
  bool hasResponse = false;
};

class ResourceNodeModel : public TreeViewBaseModel<ResourceNode> {
    Q_OBJECT
public:
    explicit ResourceNodeModel(QObject* parent = nullptr);
 ResourceNode* FindNode(const QString& path);

public slots:
    void OnGetChildInfo(const QModelIndex& index,
                        const pb::GetChildDirRsp& rsp);

    void OnGetContent(const QModelIndex& index,
                      const pb::GetUriContentRsp& rsp);

private:
    int columnCount(const QModelIndex& parent = QModelIndex()) const override {
      return 2;
    }

    QVariant OnShowData(const QModelIndex& index, int role,
                        ResourceNode* node) const override;


private:
    ResourceNode* FindChildNode(ResourceNode* parent, const QString& path);

private:
    void InitTypeIcon();
    static QMap<QString, QIcon> type_icon_map_;
};

#endif // RESOURCENODE_H
