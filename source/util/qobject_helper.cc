#include "qobject_helper.h"
#include <QQmlEngine>
#include <QQmlContext>
#include <QDirIterator>
#include <QFile>
#include <QUrl>
#include <QWidget>

static int s_unique_id_index = 0;
QString QObjectHelper::ObjectName(const QObject* obj) {
  if (auto widget = qobject_cast<const QWidget*>(obj)) {
    return widget->objectName();
  }

  QQmlContext* ctx = QQmlEngine::contextForObject(obj);
  QString name;
  do {
    if (!ctx || !ctx->engine())
      break;
    name = ctx->nameForObject(obj);
  } while (false);
  if (name.isEmpty()) {
    name = QString::number(reinterpret_cast<uint64_t>(obj), 16);
  }
  return name;
}

bool QObjectHelper::IsInRect(const QPointF& src,
                             const QPointF& left_top,
                             const QPointF& right_bottom) {
  if ((src.x() >= left_top.x()) && (src.x() <= right_bottom.x()) && 
    (src.y() >= left_top.y()) && (src.y() <= right_bottom.y())){
    return true;
  }
  return false;
}

int QObjectHelper::GenerateUniqueId() {
  return ++s_unique_id_index;
}

QByteArray QObjectHelper::ReadUriContent(const QString& uri) {
  QString path;
  if (uri.startsWith("qrc:")) {
    path = uri.mid(3);
  } else if (uri.startsWith("file:")) {
    path = QUrl(uri).toLocalFile();
  } else {
    path = uri;
  }
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    return QByteArray();
  }
  return file.readAll();
}

bool QObjectHelper::SaveFile(const QString& path, const void* buffer, int len) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return false;
  }
  file.write((const char*)buffer, len);
  return true;
}

std::string QObjectHelper::SerialPb(const ::google::protobuf::Message& pb) {
  std::string serial;
  pb.SerializeToString(&serial);
  return serial;
}

void QObjectHelper::EnumDir(const QString& dir,
    std::function<void(FileInfo&& file_info, bool& continue_enum)> delegate) {
  QDirIterator qdir(dir, QDir::NoFilter);
  while (qdir.hasNext()) {
    QFileInfo file(qdir.next());
    FileInfo file_info;
    file_info.is_file = file.isFile();
    file_info.file_path = file.absoluteFilePath();
    file_info.file_size = file.size();
    bool continue_enum = true;
    delegate(std::move(file_info), continue_enum);
    if (!continue_enum) {
      break;
    }
  }
}

QString QObjectHelper::GetFileName(const QString& file_path) {
  QFileInfo info(file_path);
  QString filename = info.fileName();
  if (filename.isEmpty()) {
    return file_path;
  }
  return filename;
}

QString QObjectHelper::MakePath(const QString& dir, const QString& name) {
  QDir qdir(dir);
  return qdir.absoluteFilePath(name);
}


bool QObjectHelper::CreateDirectoryX(const QString& dir) {
  if (QFile::exists(dir)) {
    return true;
  }
  QDir qdir;
  return qdir.mkpath(dir);
}

QString QObjectHelper::UpDir(const QString& path) {
  QDir qdir(path);
  qdir.cdUp();
  return qdir.absolutePath();
}