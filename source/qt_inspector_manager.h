#pragma once
#include <QString>
class QtInspectorManager
{
public:
  static QtInspectorManager& Instance();
 void Init(const QString& ip = "127.0.0.1", int port = 5973);
};

