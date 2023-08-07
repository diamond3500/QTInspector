#include "qt_window_node.h"
#include <QQuickWindow>
#include <QDialog>

QtWindowNode::QtWindowNode(QObject* window, int window_unique_id)
    : window_(window), window_unique_id_(window_unique_id), nodes_(window, nullptr, window_unique_id)
{}

QPixmap QtWindowNode::GrabWindow() {
  if (auto quick_window = qobject_cast<QQuickWindow*>(window_)) {
    return QPixmap::fromImage(quick_window->grabWindow());
  }
  if (auto window = qobject_cast<QDialog*>(window_)) {
    return window->grab();
  }
  return QPixmap();
}

QtObjectNode* QtWindowNode::FindQObjectByUniqueId(int unique_id) {

  return nodes_.FindQObjectByUniqueId(unique_id);
}

QString QtWindowNode::DumpToString() {
  return nodes_.DumpToString();
}


QDataStream& operator<<(QDataStream& stream, const QtWindowNode& node) {
  stream << node.nodes_;
  return stream;
}

QDataStream& operator>>(QDataStream& stream, QtWindowNode& node) {
  stream >> node.nodes_;
  return stream;
}