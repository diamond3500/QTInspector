#pragma once
#include <QObject>
#include <QString>
#include <QMap>
#include <QJsonObject>
#include <QMetaProperty>

class MetaObjectDetail {
public:
    struct MethodItem {
        QMetaMethod::MethodType _method_type;
        QMetaMethod::Access _access;
        QMetaMethod _method;
        friend QDataStream& operator<<(QDataStream& stream, const MethodItem& detail);
        friend QDataStream& operator>>(QDataStream& stream, MethodItem& detail);
    };

    struct PropertyItem {
        QVariant _value;
        QMetaProperty _prop;
        friend QDataStream& operator<<(QDataStream& stream, const PropertyItem& detail);
        friend QDataStream& operator>>(QDataStream& stream, PropertyItem& detail);
        QString toString() const {
            return _value.toString();
        }
    };

 public:
  MetaObjectDetail(const QObject* object);
  MetaObjectDetail() {}
    
  const QVariant& PropertyValueForKey(const QString& key) const;
  const QMetaProperty& PropertyForKey(const QString& key) const;

  QStringList PropertyKeys() const;
  const QString& id() const { 
    return id_;
  }

  const QMap<QString, MethodItem>& methods() const {
    return methods_;
  }

  const QString& class_name() const { 
    return class_name_; 
  }

  friend QDataStream& operator<<(QDataStream& stream, const MetaObjectDetail& detail);
  friend QDataStream& operator>>(QDataStream& stream, MetaObjectDetail& detail);

  QByteArray Serial() const;
  static MetaObjectDetail From(const QByteArray& data);

  QJsonObject DumpToJson() const;
  QString DumpToString() const;

private:
  using ObjectHandler = std::function<void(const QString&, const QVariant&)>;

  void RegisterAllObjectHandler();
  void RegisterObjectHandler(int meta_type_id, const ObjectHandler& handler);

  void HandleQQuickAnchors(const QString& property_name, const QVariant& property_value);
  void HanldeQObject(const QString& property_name,
                     const QVariant& property_value);
  void HanldeQQuickAnchorLine(const QString& property_name,
                              const QVariant& property_value);

  void HandlePropertites(const QMetaObject* meta_object, const QObject* object);
  void HandleMethod(const QMetaObject* meta_object, const QObject* object);

  void AddCustomPropertites(const QObject* object);

 private:
  QString id_;
  QString class_name_;

  QMap<QString, MethodItem> methods_;
  QMap<int, ObjectHandler> object_handler_map_;
  QMap<QString, PropertyItem> properties_;
};
