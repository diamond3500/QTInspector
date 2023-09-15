#include "qt_object_node.h"
#include <QQuickItem>
#include <QWidget>
#include <QJsonArray>
#include <QTimer>
#include <QJsonDocument>
#include "util/qobject_helper.h"

static ObjectType ObjectToType(const QObject* obj) {
    if (qobject_cast<const QQuickItem*>(obj) || qobject_cast<const QWidget*>(obj)) {
        return ObjectTypeUI;
    }
    if (qobject_cast<const QTimer*>(obj)) {
        return ObjectTypeTimer;
    }
    return ObjectTypeUnknow;
}

QtObjectNode::~QtObjectNode() {
  delete object_detail_;
  qDeleteAll(children_);
}

QtObjectNode::QtObjectNode(QObject* object, QtObjectNode* parent,int window_unique_id)
    : object_pointer_(object),
      unique_id_(QObjectHelper::GenerateUniqueId()),
      window_unique_id_(window_unique_id) {
  object_detail_ = new MetaObjectDetail(object);
  FindChildrenObject(children_, object);
}

void QtObjectNode::FindChildrenObject(QList<QtObjectNode*>& node_list, QObject* object) {  
   for (auto child : object->children()) {
        auto object_type = ObjectToType(child);
        if (object_type == ObjectTypeUnknow) {
            continue;
        }
       auto child_node = new QtObjectNode(child, this, window_unique_id_);
       child_node->object_type_ = object_type;
       node_list.push_back(child_node);
  }
}

QtObjectNode* QtObjectNode::FindQObjectByUniqueId(int unique_id) {
  if (unique_id == unique_id_) {
    return this;
  }
  for (auto child : children_) {
    auto result = child->FindQObjectByUniqueId(unique_id);
    if (result) {
      return result;
    }
  }
  return nullptr;
}

void QtObjectNode::SetProperty(const QString& prop_name,
                               const QVariant& param) {
  auto& property = object_detail_->PropertyForKey(prop_name);
  if (!property.isValid()) {
    return;
  }
  property.write(object_pointer_, param);
}

void QtObjectNode::Refresh() {
  delete object_detail_;
  qDeleteAll(children_);
  children_.clear();
  object_detail_ = new MetaObjectDetail(object_pointer_);
  FindChildrenObject(children_, object_pointer_);
}

QJsonObject QtObjectNode::DumpToJson() {
  return InnerDump(*this);
}

QString QtObjectNode::DumpToString() {
  QJsonDocument document;
  document.setObject(DumpToJson());
  return document.toJson();
}

QJsonObject QtObjectNode::InnerDump(const QtObjectNode& node) {
  QJsonObject json_object = node.object_detail_->DumpToJson();
  QJsonArray json_array;
  for (auto child_node : node.children_) {
    json_array.push_back(QtObjectNode::InnerDump(*child_node));
  }
  if (!json_array.isEmpty()) {
    json_object.insert("children", json_array);
  }
  json_object.insert("this", (qint64)&node);
  json_object.insert("parent", (qint64)node.parent_);
  return json_object;
}

QDataStream& operator<<(QDataStream& stream, const QtObjectNode& node) {
  stream << node.unique_id_ << *node.object_detail_ << node.window_unique_id_ << node.object_type_;
  stream << static_cast<int>(node.children_.size());
  for (auto child : node.children_) {
    stream << *child;
  }
  return stream;
}

QDataStream& operator>>(QDataStream& stream, QtObjectNode& node) {
  node.object_detail_ = new MetaObjectDetail();
  stream >> node.unique_id_ >> *node.object_detail_ >> node.window_unique_id_ >> node.object_type_;
  qint32 children_count = 0;
  stream >> children_count;
  for (qint32 i = 0; i < children_count; i++) {
    auto child_node = new QtObjectNode(&node);
    stream >> *child_node;
    node.children_.push_back(child_node);
  }
  return stream;
}
