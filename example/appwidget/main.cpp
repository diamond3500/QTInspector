#include "dialog.h"
#include "qt_inspector_manager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
  QtInspectorManager::Instance().Init();
    Dialog w;
    w.show();
    return a.exec();
}
