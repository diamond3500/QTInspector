#ifndef TREEVIEWBASEMODEL_H
#define TREEVIEWBASEMODEL_H

#include <memory>
#include <QAbstractItemModel>

template<class Node>
class TreeViewBaseModel : public QAbstractItemModel
{
public:
    explicit TreeViewBaseModel(QObject* parent = nullptr) {}

    // Basic functionality:
    QModelIndex index(int row, int column,
        const QModelIndex& parent = QModelIndex()) const override {
      if (!hasIndex(row, column, parent)) {
        return QModelIndex();
      }
      Node* parent_node  = nullptr;
      if (parent.isValid()) {
          parent_node = static_cast<Node*>(parent.internalPointer());
      } else {
          parent_node = root_node_.get();
      }
      Node* child_item = parent_node->children().at(row);
      if (child_item) {
          return createIndex(row, column, child_item);
      } else {
          return QModelIndex();
      }
    }
    QModelIndex parent(const QModelIndex& child) const override {
      if (!child.isValid()) {
        return QModelIndex();
      }
      auto child_node = static_cast<Node*>(child.internalPointer());
      const Node* parent_node = child_node->parent();
      if (parent_node == root_node_.get()) {
        return QModelIndex();
      }
      return createIndex(static_cast<int>(parent_node->row()), 0, (void*)parent_node);
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
      Node* parent_node  = nullptr;
      if (parent.isValid()) {
        parent_node = static_cast<Node*>(parent.internalPointer());
      } else {
        parent_node = root_node_.get();
      }

      return (int)parent_node->children().size();
    }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override {
      return 1;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
      if (!index.isValid()) {
        return QVariant();
      }
      auto node = static_cast<Node*>(index.internalPointer());
      return OnShowData(index, role, node);
    }

    void SetHeader(const QStringList& headers) {
      headers_ = headers;
    }

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override {
      if (section >= (int)headers_.size()) {
        return QVariant();
      }
      if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        return headers_.at(section);
      }
      return QVariant();
    }

protected:  
  virtual QVariant OnShowData(const QModelIndex& index, int role,
                             Node* node) const = 0;


protected:
  std::unique_ptr<Node> root_node_;
  QStringList headers_;
};

#endif // TREEVIEWBASEMODEL_H

