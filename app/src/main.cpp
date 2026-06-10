#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "ota_model.hpp"
#include "vehicle_model.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    // The native macOS style does not allow customizing control contentItems,
    // which the dark dashboard theme relies on. Basic is fully customizable and
    // renders identically across platforms.
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    VehicleModel vehicle;
    OtaModel ota;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("vehicle"), &vehicle);
    engine.rootContext()->setContextProperty(QStringLiteral("ota"), &ota);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);

    engine.loadFromModule("EvCockpit", "Main");

    return app.exec();
}
