#include "plugin.h"
#include "mainwindow.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qmlRegisterType<Plugin>("Plugin", 1, 0, "Plugin");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qmlFiles/main.qml")));

    return app.exec();
}
