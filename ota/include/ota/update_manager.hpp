#pragma once

#include <string>

namespace ota {

// Stages of an A/B over-the-air update. The pack runs from one of two slots;
// an update is written to the inactive slot and only becomes active after a
// post-install health check passes. Any failure leaves the active slot
// untouched (rollback), so a bad update can never brick the device.
enum class State {
    Idle,
    Downloading,
    Verifying,
    Swapping,
    HealthCheck,
    Active, // update committed, new version running on the other slot
    Failed, // update aborted, still running the previous version
};

// Deterministic, Qt-free update state machine. Advances only through step(dt)
// so it can be unit-tested without a UI or wall clock.
class UpdateManager {
public:
    explicit UpdateManager(std::string initial_version = "1.4.0");

    // Begin installing target_version to the inactive slot. No-op unless the
    // machine is Idle/Active/Failed (i.e. not mid-update).
    void requestUpdate(const std::string& target_version);

    // Advance the update by dt seconds.
    void step(double dt);

    // Return to Idle, keeping whatever slot is currently active.
    void reset();

    // Fault injection for demos.
    void setDownloadShouldFail(bool v) { download_fail_ = v; }
    void setChecksumShouldFail(bool v) { checksum_fail_ = v; }
    void setHealthShouldFail(bool v) { health_fail_ = v; }

    // Outputs.
    State state() const { return state_; }
    double progress() const { return progress_; } // 0..1 during Downloading
    char activeSlot() const { return active_slot_; }
    const std::string& activeVersion() const;
    const std::string& targetVersion() const { return target_; }
    const std::string& message() const { return message_; }
    bool busy() const;

private:
    struct Slot {
        std::string version;
    };

    Slot& active() { return active_slot_ == 'A' ? slot_a_ : slot_b_; }
    Slot& inactive() { return active_slot_ == 'A' ? slot_b_ : slot_a_; }
    const Slot& active() const { return active_slot_ == 'A' ? slot_a_ : slot_b_; }

    void enter(State s);
    void fail(const std::string& reason);

    Slot slot_a_;
    Slot slot_b_;
    char active_slot_ = 'A';

    State state_ = State::Idle;
    double progress_ = 0.0;
    double elapsed_ = 0.0; // time spent in the current non-download phase
    std::string target_;
    std::string message_;

    bool download_fail_ = false;
    bool checksum_fail_ = false;
    bool health_fail_ = false;

    static constexpr double kDownloadRate = 0.4; // fraction per second
    static constexpr double kPhaseSeconds = 1.0; // dwell per verify/swap/health
};

const char* toString(State s);

} // namespace ota
