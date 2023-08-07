#pragma once
#include <QString>
#include <QObject>
#include <QPointF>
#include <google/protobuf/message.h>
class QObjectHelper {
 public:
  static QString ObjectName(const QObject* obj);
  static bool IsInRect(const QPointF& src,
                       const QPointF& left_top,
                       const QPointF& right_bottom);
  static int GenerateUniqueId();

  static QByteArray ReadUriContent(const QString& uri);

  static bool SaveFile(const QString& path, const void* buffer, int len);

  static std::string SerialPb(const ::google::protobuf::Message& pb);

  struct FileInfo {
    QString file_path;
    int64_t file_size = 0;
    bool is_file = true;
  };

  static void EnumDir(
      const QString& dir,
      std::function<void(FileInfo&& file_info, bool& continue_enum)> delegate);

  static QString GetFileName(const QString& file_path);

  static QString MakePath(const QString& dir, const QString& name);

  static bool CreateDirectoryX(const QString& dir);

   static QString UpDir(const QString& path);
};