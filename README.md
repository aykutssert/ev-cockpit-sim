# ev-cockpit-sim

A simulated electric-vehicle cockpit: a battery management system (BMS) model, a Qt/QML infotainment dashboard that visualizes it live, and an over-the-air (OTA) update flow with A/B partitions and rollback.

## What it is

Three layers that mirror how a real EV's software is structured:

- A **BMS simulation core** - a battery pack of individual cells with voltage, temperature, and state-of-charge dynamics, charge/discharge behavior, and injectable fault modes (overtemperature, cell imbalance, overvoltage). Plain C++, no UI.
- An **infotainment dashboard** - a Qt/QML HMI showing speed, range, a state-of-charge gauge, per-cell battery health, and climate/media controls, updating live from the simulation.
- An **OTA update flow** - a version manifest, a download with progress, an A/B partition swap, an integrity check, and automatic rollback when an update fails its post-install health check.

The point is to exercise the parts that matter in automotive HMI and EV platform software: a real state model driving a real UI through Qt's property system, modeled failure modes (not just the happy path), and the update/rollback state machine that keeps a software-defined vehicle recoverable.

## Architecture

```
+------------------------+         +---------------------------+
|  BMS Simulation Core   |  C++    |   Qt/QML Infotainment     |
|  (plain C++17)         | <-----> |   Dashboard               |
|  - cells, SoC, temp    | Q_PROP  |  - SoC gauge, range       |
|  - charge / discharge  | signals |  - per-cell battery view  |
|  - fault injection     |         |  - climate / media        |
+------------------------+         +-------------+-------------+
                                                 |
                                                 v
                                   +---------------------------+
                                   |   OTA Update Manager      |
                                   |  - manifest, download     |
                                   |  - A/B swap, verify        |
                                   |  - rollback on failure     |
                                   +---------------------------+
```

- **BMS core**: Qt-free, deterministic `step(dt)`, unit-tested in isolation.
- **Backend bridge**: C++ `QObject`s expose simulation state to QML via `Q_PROPERTY` and change signals; a timer advances the simulation and drives the UI.
- **OTA manager**: a C++ state machine (Idle -> Downloading -> Verifying -> Swapping -> Health-check -> Active | RolledBack) surfaced to QML.

## Tech Stack

- **Language**: C++17
- **UI**: Qt 6 / QML (Qt Quick)
- **Build**: CMake (`find_package(Qt6)`)
- **Tests**: lightweight C++ unit tests for the BMS core (no GUI dependency)
- **Platform**: macOS native app (no VM, no cross-compilation)

## Decisions

Architectural choices get recorded here once made, with the reason.

| Decision | Choice | Reason |
|---|---|---|
| UI framework | Qt 6 / QML | Industry standard for automotive HMI (used across OEM clusters and infotainment, including QML-based systems at major EV makers). Native macOS support, mature property/signal binding to a C++ backend, good fit for a live-updating dashboard. |
| Simulation core separated from UI | Plain C++17 BMS library, Qt only in the presentation layer | Keeps the model unit-testable without an event loop, and keeps battery/fault logic out of QML. The same core could drive a different frontend or a headless test. |
| OTA update model | A/B dual-slot with a post-install health check and automatic rollback | Mirrors how real software-defined vehicles update safely: the new image is written to the inactive slot and only committed if it passes a health check, so a failed update never leaves the device unbootable. The state machine is the interesting part, so it lives in a Qt-free, unit-tested core like the BMS. |
| Qt Quick Controls style | Basic (set via QQuickStyle) instead of the native macOS style | The dark dashboard theme customizes control contentItems (checkbox/switch labels), which the native macOS style forbids. Basic is fully customizable and renders identically across platforms. |

## Roadmap

### Phase 0 - Toolchain
- [x] Verify Qt6 builds and launches a minimal QML window on macOS via CMake (critical assumption)

### Phase 1 - BMS Simulation Core
- [x] Battery pack model: N cells, per-cell voltage + temperature, pack-level SoC
- [x] Charge / discharge dynamics driven by a deterministic `step(dt)`
- [x] Fault injection: overtemperature, cell imbalance, overvoltage
- [x] Unit tests for SoC integration, fault thresholds, charge/discharge

### Phase 2 - Infotainment Dashboard
- [x] C++ backend exposing simulation state to QML (Q_PROPERTY + signals)
- [x] QML dashboard: SoC gauge, range, speed, per-cell battery view
- [x] Live updates: timer-driven simulation reflected in the UI
- [x] Fault state surfaced visually (warning when a cell faults)

### Phase 3 - OTA Update Flow
- [x] OTA state machine: manifest, download, verify, A/B swap, health check, rollback
- [x] QML UI to trigger an update and show progress + current state
- [x] Fault injection: failed download / bad checksum triggers rollback

## Results

Running `ev_cockpit_sim` opens a two-tab cockpit:

- **Dashboard** - an SoC gauge, speed, range, pack voltage, max cell
  temperature, and current, plus a 96-cell grid coloured by state of charge.
  The throttle slider and charge switch drive the BMS sim live; the
  fault-injection buttons trip overtemperature and cell imbalance, which surface
  as a red cell and a fault banner.
- **Software** - the OTA flow. "Install" runs the update through Download ->
  Verify -> Write slot -> Health check; on success the active slot flips
  (A -> B) and the version advances (e.g. 1.4.0 -> 1.5.0). The fault-injection
  checkboxes force a failed download, a checksum mismatch, or a failed health
  check; in every failure case the active slot is left untouched and the UI
  reports a rollback to the previous version.

The two simulation cores (`bms`, `ota`) are covered by 14 unit tests run via
CTest, independent of the GUI.

**Limitations and simplifications:**
- The cell OCV-SoC relationship is linear; real cells have a nonlinear curve
  with a flat middle plateau.
- The pack is modeled as cells in series only (no parallel groups), so the
  "pack current" values are illustrative, not a real 5 Ah cell's C-rate.
- The OTA download, checksum, and health check are simulated timers and flags,
  not real image transfer or verification - the point is the A/B + rollback
  state machine, not cryptographic integrity.
- Cell cooling is tuned so overtemperature comes from fault injection rather
  than normal driving load.

## Success Criteria

- A battery model with multiple cells runs a deterministic simulation, testable without the GUI
- The Qt/QML dashboard shows live vehicle and battery state driven by that model
- At least one fault mode is injectable and visibly reflected in the UI
- The OTA flow performs an A/B update and rolls back automatically on a simulated failure
- Known limitations and simplifications are documented, not hidden

## Status

All phases done. Qt6/QML cockpit with a tested BMS simulation core, a live
dashboard, and an A/B OTA update flow with rollback. 14 unit tests passing.
