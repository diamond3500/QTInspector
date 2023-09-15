#include "object_node_model.h"
#include <QIcon>

QMap<QString, QIcon> ObjectNodeModel::type_icon_map_;
ObjectNodeModel::ObjectNodeModel(std::unique_ptr<QtObjectNode> root_node) {
  root_node_ = std::move(root_node);
  InitTypeIcon();
}

void ObjectNodeModel::InitTypeIcon() {
  if (type_icon_map_.empty()) {
    type_icon_map_["QQuickText"] = QIcon(":/icons/classes/qtquick2/text-icon16.png");
    type_icon_map_["QQuickRectangle"] = QIcon(":/icons/classes/qtquick2/rect-icon16.png");
    type_icon_map_["QQuickImage"] = QIcon(":/icons/classes/qtquick2/image-icon16.png");
    type_icon_map_["QPushButton"] = QIcon(":/icons/classes/qtwidgets/pushbutton.png");
    type_icon_map_["QLabel"] = QIcon(":/icons/classes/qtwidgets/label.png");
    type_icon_map_["QComboBox"] = QIcon(":/icons/classes/qtwidgets/combobox.png");
    type_icon_map_["QToolButton"] = QIcon(":/icons/classes/qtwidgets/toolbutton.png");
    type_icon_map_["QRadioButton"] = QIcon(":/icons/classes/qtwidgets/radiobutton.png");
    type_icon_map_["QCheckBox"] = QIcon(":/icons/classes/qtwidgets/checkbox.png");
    type_icon_map_["QDialogButtonBox"] = QIcon(":/icons/classes/qtwidgets/dialogbuttonbox.png");
    type_icon_map_["QListView"] = QIcon(":/icons/classes/qtwidgets/listview.png");
    type_icon_map_["QTreeView"] = QIcon(":/icons/classes/qtwidgets/listview.png");
    type_icon_map_["QTableView"] = QIcon(":/icons/classes/qtwidgets/table.png");
    type_icon_map_["QTableWidget"] = QIcon(":/icons/classes/qtwidgets/tabwidget.png");
    type_icon_map_["QLCDNumber"] = QIcon(":/icons/classes/qtwidgets/lcdnumber.png");
    type_icon_map_["QProgressBar"] = QIcon(":/icons/classes/qtwidgets/progress.png");
    type_icon_map_["QDial"] = QIcon(":/icons/classes/qtwidgets/dial.png");
    type_icon_map_["QDateTimeEdit"] = QIcon(":/icons/classes/qtwidgets/datetimeedit.png");
    type_icon_map_["QTimeEdit"] = QIcon(":/icons/classes/qtwidgets/timeedit.png");
    type_icon_map_["QSpinBox"] = QIcon(":/icons/classes/qtwidgets/spinbox.png");
    type_icon_map_["QPlainTextEdit"] = QIcon(":/icons/classes/qtwidgets/plaintextedit.png");
    type_icon_map_["QTextEdit"] = QIcon(":/icons/classes/qtwidgets/textedit.png");
    type_icon_map_["QLineEdit"] = QIcon(":/icons/classes/qtwidgets/lineedit.png");
  }
}

QVariant ObjectNodeModel::OnShowData(const QModelIndex& index,
                               int role,
                               QtObjectNode* node) const {
  auto object_detail = node->object_detail();
  if (!object_detail) {
    return QVariant();
  }
  switch (role) {
    case Qt::DisplayRole: {
      return QVariant(QString("%1[%2]")
                          .arg(object_detail->id())
                          .arg(object_detail->class_name()));
    }
    case Qt::DecorationRole: {
      return type_icon_map_[object_detail->class_name()];
    }
    default: {
      return QVariant();
    }
  }
}

ObjectSearchProxyModel::ObjectSearchProxyModel(QObject* parent) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
  setRecursiveFilteringEnabled(true);
#endif
}

void ObjectSearchProxyModel::SetSearchText(const QString& text, bool flush) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  search_text_ = text;
  if (flush) {
    invalidateRowsFilter();
  }
#endif
}

void ObjectSearchProxyModel::SetSearchType(ObjectType type, bool select, bool flush) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  if (select) {
     filter_type_ |= type;
  } else {
      filter_type_ &= ~type;
  }
  if (flush) {
    invalidateRowsFilter();
  }
#endif
}

bool ObjectSearchProxyModel::filterAcceptsRow(int source_row,
    const QModelIndex& source_parent) const {
  QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
  auto object_node = static_cast<QtObjectNode*>(index.internalPointer());
  auto object_detail = object_node->object_detail();
  if (!object_detail) {
    return true;
  }
  if ((object_node->object_type() & filter_type_) == 0) {
    return false;
  }
 if (search_text_.isEmpty()) {
  return true;
 }
  if (object_detail->class_name().contains(search_text_, Qt::CaseInsensitive)) {
    return true;
  }
  if (object_detail->id().contains(search_text_, Qt::CaseInsensitive)) {
    return true;
  }
  return false;
}
