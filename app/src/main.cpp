#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "vehicle_model.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    VehicleModel vehicle;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("vehicle"), &vehicle);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);

    engine.loadFromModule("EvCockpit", "Main");

    return app.exec();
}
