#include "vehicle_model.hpp"

#include <algorithm>

VehicleModel::VehicleModel(QObject* parent) : QObject(parent) {
    cells_.refresh(pack_);
    connect(&timer_, &QTimer::timeout, this, &VehicleModel::tick);
    timer_.start(kTickMs);
}

double VehicleModel::soc() const { return pack_.packSoc(); }

double VehicleModel::range() const { return kFullRangeKm * pack_.packSoc(); }

QString VehicleModel::faultText() const {
    QStringList parts;
    for (const auto& f : pack_.faults()) {
        switch (f.type) {
        case bms::FaultType::OverTemperature:
            parts << QStringLiteral("Overtemp cell %1").arg(f.cell_index);
            break;
        case bms::FaultType::CellImbalance:
            parts << QStringLiteral("Cell imbalance");
            break;
        case bms::FaultType::OverVoltage:
            parts << QStringLiteral("Overvoltage cell %1").arg(f.cell_index);
            break;
        case bms::FaultType::UnderVoltage:
            parts << QStringLiteral("Undervoltage cell %1").arg(f.cell_index);
            break;
        }
    }
    return parts.join(QStringLiteral(", "));
}

void VehicleModel::setThrottle(double t) {
    throttle_ = std::clamp(t, 0.0, 1.0);
    emit changed();
}

void VehicleModel::setCharging(bool on) {
    charging_ = on;
    if (on) {
        throttle_ = 0.0;
    }
    emit changed();
}

void VehicleModel::injectOvertemp() {
    // Force a representative cell well past the overtemp threshold.
    pack_.injectOverTemperature(7, 70.0);
}

void VehicleModel::injectImbalance() {
    // Add a parasitic drain so one cell drifts away from the pack over time.
    pack_.injectImbalance(13, 8.0);
}

void VehicleModel::clearFaults() { pack_.clearInjections(); }

void VehicleModel::tick() {
    if (charging_) {
        // Stationary fast-charge at a fixed rate; current is negative (into the
        // pack). Stop drawing once effectively full.
        speed_ += (0.0 - speed_) * 0.3;
        pack_.setCurrent(pack_.packSoc() < 0.999 ? -60.0 : 0.0);
    } else {
        // Speed eases toward a throttle-proportional target; discharge current
        // grows with throttle.
        const double target = throttle_ * 180.0; // km/h
        speed_ += (target - speed_) * 0.1;
        if (speed_ < 0.1) {
            speed_ = 0.0;
        }
        pack_.setCurrent(30.0 + throttle_ * 170.0); // 30..200 A
    }

    pack_.step(kSimDtSeconds);
    cells_.refresh(pack_);
    emit changed();
}
