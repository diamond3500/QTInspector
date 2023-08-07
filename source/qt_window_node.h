#pragma once
#include "qt_object_node.h"

class QtWindowNode {
 public:
  QtWindowNode(QObject* window, int unique_id);
  QtWindowNode() : nodes_(nullptr) {}
  QPixmap GrabWindow();

  const QtObjectNode* root_node() const {
      return &nodes_;
  }

  int window_unique_id() { 
    return window_unique_id_;
  }

  QtObjectNode* FindQObjectByUniqueId(int unique_id);

  friend QDataStream& operator<<(QDataStream& stream, const QtWindowNode& node);
  friend QDataStream& operator>>(QDataStream& stream, QtWindowNode& node);

  QString DumpToString();

private:
  QObject* window_ = nullptr;
  int window_unique_id_ = 0;
  QtObjectNode nodes_;
};
