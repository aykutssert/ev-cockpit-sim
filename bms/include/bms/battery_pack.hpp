#pragma once

#include <vector>

namespace bms {

// Per-cell snapshot, in SI-ish units (V, deg C, fraction).
struct CellState {
    double soc = 1.0;          // state of charge, 0..1
    double voltage = 4.2;      // terminal voltage under load, volts
    double temperature = 25.0; // degrees Celsius
};

enum class FaultType {
    OverTemperature,
    CellImbalance,
    OverVoltage,
    UnderVoltage,
};

struct Fault {
    FaultType type;
    int cell_index; // index of the offending cell, or -1 for a pack-level fault
};

// Tunable pack parameters. Defaults model a ~96S EV pack of 5 Ah cells.
struct PackConfig {
    int cell_count = 96;
    double cell_capacity_ah = 5.0;
    double internal_resistance_ohm = 0.01; // per cell
    double ambient_temp_c = 25.0;
    double thermal_mass = 800.0; // J per deg C, per cell
    double cooling_coeff = 16.0; // W per deg C, per cell (convective loss to ambient)

    // Fault thresholds.
    double overtemp_c = 55.0;
    double overvoltage_v = 4.25;
    double undervoltage_v = 2.80;
    double imbalance_soc = 0.15; // max-min SoC spread that counts as imbalance
};

// Deterministic battery-pack simulation. The model is Qt-free and advances
// only through step(dt) - it never reads a wall clock - so the same inputs
// always produce the same outputs and it can be unit-tested without a UI.
//
// Sign convention for current: positive = discharge (driving), negative =
// charge.
class BatteryPack {
public:
    explicit BatteryPack(PackConfig config = {});

    // Advance the simulation by dt seconds.
    void step(double dt_seconds);

    // Inputs.
    void setCurrent(double amps);
    double current() const { return current_; }

    // Fault injection for demos. injectOverTemperature pins a cell's
    // temperature; injectImbalance adds a continuous parasitic drain (amps) to
    // one cell so its SoC drifts away from the pack over time.
    void injectOverTemperature(int cell_index, double temp_c);
    void injectImbalance(int cell_index, double drain_amps);
    void clearInjections();

    // Outputs.
    int cellCount() const { return static_cast<int>(cells_.size()); }
    const CellState& cell(int i) const { return cells_[i]; }
    double packVoltage() const; // sum of cell terminal voltages
    double packSoc() const;     // mean cell SoC, 0..1
    double maxCellTemp() const;
    double socSpread() const;   // max - min cell SoC

    std::vector<Fault> faults() const;
    bool hasFault() const;

private:
    PackConfig cfg_;
    std::vector<CellState> cells_;
    std::vector<double> forced_temp_; // NaN = not forced
    std::vector<double> drain_amps_;  // parasitic per-cell drain
    double current_ = 0.0;

    double ocv(double soc) const; // open-circuit voltage for a given SoC
};

} // namespace bms
