#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include "bms/battery_pack.hpp"
#include "cell_model.hpp"

// The C++ backend the dashboard binds to. It owns the BMS simulation, drives it
// on a timer, and exposes vehicle/battery state to QML through Q_PROPERTY. QML
// is presentation only; all decisions (driving model, fault thresholds) live
// here. State changes are published through a single changed() signal, which is
// enough for a dashboard refreshing at the tick rate.
class VehicleModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(double soc READ soc NOTIFY changed)
    Q_PROPERTY(double speed READ speed NOTIFY changed)
    Q_PROPERTY(double range READ range NOTIFY changed)
    Q_PROPERTY(double packVoltage READ packVoltage NOTIFY changed)
    Q_PROPERTY(double maxTemp READ maxTemp NOTIFY changed)
    Q_PROPERTY(double current READ current NOTIFY changed)
    Q_PROPERTY(double throttle READ throttle WRITE setThrottle NOTIFY changed)
    Q_PROPERTY(bool charging READ charging NOTIFY changed)
    Q_PROPERTY(bool faultActive READ faultActive NOTIFY changed)
    Q_PROPERTY(QString faultText READ faultText NOTIFY changed)
    Q_PROPERTY(CellModel* cells READ cells CONSTANT)

public:
    explicit VehicleModel(QObject* parent = nullptr);

    double soc() const;
    double speed() const { return speed_; }
    double range() const;
    double packVoltage() const { return pack_.packVoltage(); }
    double maxTemp() const { return pack_.maxCellTemp(); }
    double current() const { return pack_.current(); }
    double throttle() const { return throttle_; }
    bool charging() const { return charging_; }
    bool faultActive() const { return pack_.hasFault(); }
    QString faultText() const;
    CellModel* cells() { return &cells_; }

    Q_INVOKABLE void setThrottle(double t);
    Q_INVOKABLE void setCharging(bool on);
    Q_INVOKABLE void injectOvertemp();
    Q_INVOKABLE void injectImbalance();
    Q_INVOKABLE void clearFaults();

signals:
    void changed();

private:
    void tick();

    static constexpr double kFullRangeKm = 450.0;
    static constexpr double kSimDtSeconds = 1.0; // sim seconds advanced per tick
    static constexpr int kTickMs = 100;

    bms::BatteryPack pack_;
    CellModel cells_;
    QTimer timer_;

    double throttle_ = 0.0; // 0..1
    double speed_ = 0.0;    // km/h
    bool charging_ = false;
};
