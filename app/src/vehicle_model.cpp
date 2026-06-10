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
    // Aggregate by fault type so a pack-wide condition (e.g. every cell
    // undervolting at 0% SoC) reads as "Undervoltage x96" instead of listing
    // all 96 cells and overflowing the banner.
    struct Group {
        int count = 0;
        int first_cell = -1;
    };
    Group overtemp, imbalance, overvolt, undervolt;
    auto add = [](Group& g, int cell) {
        if (g.count == 0) {
            g.first_cell = cell;
        }
        ++g.count;
    };
    for (const auto& f : pack_.faults()) {
        switch (f.type) {
        case bms::FaultType::OverTemperature:
            add(overtemp, f.cell_index);
            break;
        case bms::FaultType::CellImbalance:
            add(imbalance, f.cell_index);
            break;
        case bms::FaultType::OverVoltage:
            add(overvolt, f.cell_index);
            break;
        case bms::FaultType::UnderVoltage:
            add(undervolt, f.cell_index);
            break;
        }
    }

    auto label = [](const QString& name, const Group& g) {
        if (g.count == 0) {
            return QString();
        }
        if (g.count == 1) {
            return g.first_cell >= 0 ? QStringLiteral("%1 cell %2").arg(name).arg(g.first_cell)
                                     : name;
        }
        return QStringLiteral("%1 x%2").arg(name).arg(g.count);
    };

    QStringList parts;
    for (const QString& s : {label(QStringLiteral("Overtemp"), overtemp),
                             label(QStringLiteral("Overvoltage"), overvolt),
                             label(QStringLiteral("Undervoltage"), undervolt),
                             label(QStringLiteral("Cell imbalance"), imbalance)}) {
        if (!s.isEmpty()) {
            parts << s;
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
