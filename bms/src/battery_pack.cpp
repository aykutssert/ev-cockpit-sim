#include "bms/battery_pack.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace bms {

namespace {
constexpr double kNan = std::numeric_limits<double>::quiet_NaN();

// Linear open-circuit-voltage curve: 3.0 V empty, 4.2 V full. A real cell's
// OCV-SoC curve is nonlinear; this is a deliberate simplification (documented
// in the README) that keeps the model readable while still moving voltage with
// charge.
constexpr double kOcvEmpty = 3.0;
constexpr double kOcvFull = 4.2;
} // namespace

BatteryPack::BatteryPack(PackConfig config) : cfg_(config) {
    const int n = std::max(1, cfg_.cell_count);
    cells_.assign(static_cast<size_t>(n), CellState{});
    forced_temp_.assign(static_cast<size_t>(n), kNan);
    drain_amps_.assign(static_cast<size_t>(n), 0.0);
    for (auto& c : cells_) {
        c.soc = 1.0;
        c.temperature = cfg_.ambient_temp_c;
        c.voltage = ocv(c.soc);
    }
}

double BatteryPack::ocv(double soc) const {
    return kOcvEmpty + (kOcvFull - kOcvEmpty) * std::clamp(soc, 0.0, 1.0);
}

void BatteryPack::setCurrent(double amps) { current_ = amps; }

void BatteryPack::injectOverTemperature(int cell_index, double temp_c) {
    if (cell_index >= 0 && cell_index < cellCount()) {
        forced_temp_[static_cast<size_t>(cell_index)] = temp_c;
    }
}

void BatteryPack::injectImbalance(int cell_index, double drain_amps) {
    if (cell_index >= 0 && cell_index < cellCount()) {
        drain_amps_[static_cast<size_t>(cell_index)] = drain_amps;
    }
}

void BatteryPack::clearInjections() {
    std::fill(forced_temp_.begin(), forced_temp_.end(), kNan);
    std::fill(drain_amps_.begin(), drain_amps_.end(), 0.0);
}

void BatteryPack::step(double dt_seconds) {
    if (dt_seconds <= 0.0) {
        return;
    }
    const double r = cfg_.internal_resistance_ohm;
    const double cap_coulombs = cfg_.cell_capacity_ah * 3600.0;

    for (size_t i = 0; i < cells_.size(); ++i) {
        CellState& c = cells_[i];

        // Same series current flows through every cell, plus any injected
        // parasitic drain unique to this cell.
        const double cell_current = current_ + drain_amps_[i];

        // Coulomb counting: discharge (positive current) lowers SoC.
        c.soc -= cell_current * dt_seconds / cap_coulombs;
        c.soc = std::clamp(c.soc, 0.0, 1.0);

        // Terminal voltage sags under discharge, rises under charge.
        c.voltage = ocv(c.soc) - cell_current * r;

        // Thermal model: I^2 R heating, Newtonian cooling to ambient. A forced
        // temperature (fault injection) overrides the dynamics for that cell.
        if (std::isnan(forced_temp_[i])) {
            const double heat_w = cell_current * cell_current * r;
            const double cool_w = cfg_.cooling_coeff * (c.temperature - cfg_.ambient_temp_c);
            c.temperature += (heat_w - cool_w) * dt_seconds / cfg_.thermal_mass;
        } else {
            c.temperature = forced_temp_[i];
        }
    }
}

double BatteryPack::packVoltage() const {
    double sum = 0.0;
    for (const auto& c : cells_) {
        sum += c.voltage;
    }
    return sum;
}

double BatteryPack::packSoc() const {
    if (cells_.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (const auto& c : cells_) {
        sum += c.soc;
    }
    return sum / static_cast<double>(cells_.size());
}

double BatteryPack::maxCellTemp() const {
    double m = -std::numeric_limits<double>::infinity();
    for (const auto& c : cells_) {
        m = std::max(m, c.temperature);
    }
    return m;
}

double BatteryPack::socSpread() const {
    if (cells_.empty()) {
        return 0.0;
    }
    double lo = cells_[0].soc;
    double hi = cells_[0].soc;
    for (const auto& c : cells_) {
        lo = std::min(lo, c.soc);
        hi = std::max(hi, c.soc);
    }
    return hi - lo;
}

std::vector<Fault> BatteryPack::faults() const {
    std::vector<Fault> out;
    for (int i = 0; i < cellCount(); ++i) {
        const CellState& c = cells_[static_cast<size_t>(i)];
        if (c.temperature > cfg_.overtemp_c) {
            out.push_back({FaultType::OverTemperature, i});
        }
        if (c.voltage > cfg_.overvoltage_v) {
            out.push_back({FaultType::OverVoltage, i});
        }
        if (c.voltage < cfg_.undervoltage_v) {
            out.push_back({FaultType::UnderVoltage, i});
        }
    }
    if (socSpread() > cfg_.imbalance_soc) {
        out.push_back({FaultType::CellImbalance, -1});
    }
    return out;
}

bool BatteryPack::hasFault() const { return !faults().empty(); }

} // namespace bms
