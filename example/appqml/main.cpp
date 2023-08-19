#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "qt_inspector_manager.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
QtInspectorManager::Instance().Init();
    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/appqml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);
    
    return app.exec();
}
