#include "qt_inspector_manager.h"
#include "qt_inspector_manager_impl.h"


QtInspectorManager& QtInspectorManager::Instance()
{
  static QtInspectorManager s_instance;
  return s_instance;
}

void QtInspectorManager::Init(const QString& ip, int port) {
  QtInspectorManagerImpl::Instance().Init(ip, port);
}
