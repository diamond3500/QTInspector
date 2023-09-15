#pragma once
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "qt_object_node.h"
#include "tree_view_base_model.h"
//https://blog.csdn.net/sinat_31608641/article/details/125965595


class ObjectNodeModel : public TreeViewBaseModel<QtObjectNode> {
  Q_OBJECT
 public:
  ObjectNodeModel(std::unique_ptr<QtObjectNode> root_node);

private:
  void InitTypeIcon();
 QVariant OnShowData(const QModelIndex& index,
                     int role,
                     QtObjectNode* node) const override;

private:
  static QMap<QString, QIcon> type_icon_map_;
};

class ObjectSearchProxyModel : public QSortFilterProxyModel {
  Q_OBJECT

public:
  explicit ObjectSearchProxyModel(QObject* parent = nullptr);
  void SetSearchText(const QString& text, bool flush);
  void SetSearchType(ObjectType type, bool select, bool flush);

protected:
 bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

 private:
 QString search_text_;
  int filter_type_ = 0;
};
