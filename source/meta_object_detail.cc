#include "meta_object_detail.h"
#include "util/qobject_helper.h"
#include <QDataStream>
#include <QJsonDocument>
#include <QQuickItem>
#include <QWidget>
#include <functional>

#define REGISTER_OBJECT_HANDLE(type, handler) \
  RegisterObjectHandler(qMetaTypeId<type>(), std::bind(handler, this, std::placeholders::_1, std::placeholders::_2))

MetaObjectDetail::MetaObjectDetail(const QObject* object) {
  AddCustomPropertites(object);
  RegisterAllObjectHandler();
  id_ = QObjectHelper::ObjectName(object);
  const QMetaObject* meta_object = object->metaObject();
  if (nullptr == meta_object) {
    return;
  }
  class_name_ = meta_object->className();
  int property_count = meta_object->propertyCount();
  for (int i = 0; i < property_count; i++) {
    QMetaProperty property = meta_object->property(i);
    QString property_name = property.name();
    property_map_[property_name] = property;
    QVariant property_value = property.read(object);
    int meta_type_id = property_value.type();
    if ((meta_type_id < QMetaType::User) &&
        (meta_type_id != qMetaTypeId<QObject*>())) {
      properties_[property_name] = property_value;
    } else {
      auto iter = object_handler_map_.find(meta_type_id);
      if (iter != object_handler_map_.end()) {
        (*iter)(property_name, property_value);
      } else {
        /* qDebug() << meta_type_id << "|" << property_name << "|"
                 << property_value.typeName() << "|"
                 << property_value.toString();*/
      }
    }
  }
}

void MetaObjectDetail::AddCustomPropertites(const QObject* object) {
  if (auto quick_object = qobject_cast<const QQuickItem*>(object)) {
    QPointF left_top = quick_object->mapToItem(nullptr, QPointF(0, 0));
    properties_["screen_x"] = left_top.x();
    properties_["screen_y"] = left_top.y();
    return;
  }
  if (auto widget_object = qobject_cast<const QWidget*>(object)) {
    QPointF left_top;
    QWidget* parent_widget = widget_object->parentWidget();
    if (parent_widget) {
      while (true) {
        auto tmp_widget = parent_widget->parentWidget();
        if (!tmp_widget) {
          break;
        }
        parent_widget = tmp_widget;
      }
      left_top = widget_object->mapTo(parent_widget, QPoint(0, 0));
    }
   
    properties_["screen_x"] = left_top.x();
    properties_["screen_y"] = left_top.y();
    return;
  }
 }

void MetaObjectDetail::RegisterAllObjectHandler() {
  //REGISTER_OBJECT_HANDLE(QQuickAnchors*, &MetaObjectDetail::HandleQQuickAnchors);
  REGISTER_OBJECT_HANDLE(QObject*, &MetaObjectDetail::HanldeQObject);
  //REGISTER_OBJECT_HANDLE(QQuickAnchorLine, &MetaObjectDetail::HanldeQQuickAnchorLine);
}

const QVariant& MetaObjectDetail::PropertyValueForKey(const QString& key) const {
  static QVariant empty_property_value;
  auto iter = properties_.find(key);
  if (iter == properties_.end()) {
    return empty_property_value;
  }
  return iter.value();
}

const QMetaProperty& MetaObjectDetail::PropertyForKey(const QString& key) const {
  static QMetaProperty empty_property;
  auto iter = property_map_.find(key);
  if (iter == property_map_.end()) {
    return empty_property;
  }
  return iter.value();
}

QStringList MetaObjectDetail::PropertyKeys() const {
  return properties_.keys();
}

QJsonObject MetaObjectDetail::DumpToJson() const {
  QJsonObject json_object;
  json_object.insert("id", id_);
  json_object.insert("className", class_name_);

  QJsonObject properties_json;
  foreach (const QString& key, properties_.keys()) {
    properties_json.insert(key, properties_[key].toString());
  }
  json_object.insert("properties", properties_json);
  return json_object;
}

QString MetaObjectDetail::DumpToString() const {
  QJsonDocument document;
  document.setObject(DumpToJson());
  return document.toJson();
}

void MetaObjectDetail::HandleQQuickAnchors(const QString& property_name,
                                           const QVariant& property_value) {
}

void MetaObjectDetail::HanldeQObject(const QString& property_name,
                                     const QVariant& property_value) {
}

void MetaObjectDetail::HanldeQQuickAnchorLine(const QString& property_name,
                          const QVariant& property_value) {
}

QByteArray MetaObjectDetail::Serial() const {
  QByteArray data;
  QDataStream data_stream(&data, QIODevice::WriteOnly);
  data_stream << *this;
  return data;
}

MetaObjectDetail MetaObjectDetail::From(const QByteArray& data) {
  MetaObjectDetail meta_object;
  QDataStream data_stream(data);
  data_stream >> meta_object;
  return meta_object;
}

void MetaObjectDetail::RegisterObjectHandler(int meta_type_id,
                                            const ObjectHandler& handler) {
  object_handler_map_.insert(meta_type_id, handler);
}

QDataStream& operator<<(QDataStream& stream, const MetaObjectDetail& detail) {
  stream << detail.id_ << detail.class_name_
         << detail.properties_;
  return stream;
}

QDataStream& operator>>(QDataStream& stream, MetaObjectDetail& detail) {
  stream >> detail.id_ >> detail.class_name_ >> detail.properties_;
  return stream;
}
