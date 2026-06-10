// Lightweight assert-based tests for the BMS core. No GTest dependency: a
// tiny CHECK macro counts failures and the process exits non-zero if any fail,
// which is all CTest needs.
#include "bms/battery_pack.hpp"

#include <cmath>
#include <cstdio>
#include <string>

namespace {

int g_failures = 0;

void check(bool cond, const std::string& what) {
    if (!cond) {
        std::printf("  FAIL: %s\n", what.c_str());
        ++g_failures;
    }
}

bool nearly(double a, double b, double eps = 1e-6) { return std::fabs(a - b) < eps; }

bool hasFault(const bms::BatteryPack& pack, bms::FaultType type) {
    for (const auto& f : pack.faults()) {
        if (f.type == type) {
            return true;
        }
    }
    return false;
}

// Coulomb counting: discharging a 5 Ah cell at 5 A for 360 s removes exactly
// 0.1 of SoC (5 * 360 / (5 * 3600)).
void test_discharge_soc() {
    bms::PackConfig cfg;
    cfg.cell_count = 1;
    cfg.cell_capacity_ah = 5.0;
    bms::BatteryPack pack(cfg);
    pack.setCurrent(5.0);
    for (int i = 0; i < 360; ++i) {
        pack.step(1.0);
    }
    check(nearly(pack.packSoc(), 0.9, 1e-6), "discharge 5A/360s drops SoC by 0.1");
}

// Charging (negative current) raises SoC back up.
void test_charge_soc() {
    bms::PackConfig cfg;
    cfg.cell_count = 1;
    cfg.cell_capacity_ah = 5.0;
    bms::BatteryPack pack(cfg);
    pack.setCurrent(5.0);
    for (int i = 0; i < 360; ++i) {
        pack.step(1.0); // discharge to 0.9
    }
    pack.setCurrent(-5.0);
    for (int i = 0; i < 180; ++i) {
        pack.step(1.0); // charge back up 0.05
    }
    check(nearly(pack.packSoc(), 0.95, 1e-6), "charge -5A/180s raises SoC by 0.05");
}

// Pack voltage is the series sum of cell terminal voltages, and tracks SoC.
void test_pack_voltage() {
    bms::PackConfig cfg;
    cfg.cell_count = 96;
    bms::BatteryPack pack(cfg);
    // Full pack, no load: each cell at OCV(1.0) = 4.2 V -> 403.2 V.
    check(nearly(pack.packVoltage(), 96 * 4.2, 1e-6), "full pack voltage = 96 * 4.2");
}

// Forcing a cell's temperature past the threshold raises an overtemp fault on
// that cell.
void test_overtemp_injection() {
    bms::BatteryPack pack;
    check(!pack.hasFault(), "fresh pack has no faults");
    pack.injectOverTemperature(3, 70.0);
    pack.step(1.0);
    check(hasFault(pack, bms::FaultType::OverTemperature), "injected overtemp raises fault");
    bool right_cell = false;
    for (const auto& f : pack.faults()) {
        if (f.type == bms::FaultType::OverTemperature && f.cell_index == 3) {
            right_cell = true;
        }
    }
    check(right_cell, "overtemp fault reports the injected cell index");
}

// A parasitic drain on one cell pulls its SoC away from the pack until the
// imbalance threshold trips.
void test_imbalance_injection() {
    bms::PackConfig cfg;
    cfg.cell_count = 10;
    cfg.imbalance_soc = 0.15;
    bms::BatteryPack pack(cfg);
    pack.injectImbalance(0, 5.0); // 5 A extra drain on cell 0
    check(!hasFault(pack, bms::FaultType::CellImbalance), "no imbalance at start");
    for (int i = 0; i < 600; ++i) {
        pack.step(1.0);
    }
    check(pack.socSpread() > 0.15, "drained cell creates SoC spread > threshold");
    check(hasFault(pack, bms::FaultType::CellImbalance), "imbalance fault trips");
}

// Same inputs, same outputs: the model must be deterministic.
void test_determinism() {
    auto run = []() {
        bms::PackConfig cfg;
        cfg.cell_count = 8;
        bms::BatteryPack pack(cfg);
        pack.setCurrent(20.0);
        pack.injectImbalance(2, 1.5);
        for (int i = 0; i < 1000; ++i) {
            pack.step(0.5);
        }
        return pack.packSoc() + pack.maxCellTemp() + pack.packVoltage();
    };
    check(nearly(run(), run(), 1e-12), "two identical runs produce identical state");
}

// Heavy discharge heats the pack above ambient via I^2 R.
void test_thermal_rise() {
    bms::PackConfig cfg;
    cfg.cell_count = 1;
    bms::BatteryPack pack(cfg);
    const double t0 = pack.cell(0).temperature;
    pack.setCurrent(100.0);
    for (int i = 0; i < 600; ++i) {
        pack.step(1.0);
    }
    check(pack.cell(0).temperature > t0, "sustained high current warms the cell");
}

} // namespace

int main() {
    std::printf("running bms tests...\n");
    test_discharge_soc();
    test_charge_soc();
    test_pack_voltage();
    test_overtemp_injection();
    test_imbalance_injection();
    test_determinism();
    test_thermal_rise();

    if (g_failures == 0) {
        std::printf("all bms tests passed\n");
        return 0;
    }
    std::printf("%d bms test(s) failed\n", g_failures);
    return 1;
}
