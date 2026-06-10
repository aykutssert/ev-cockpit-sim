#include "ota/update_manager.hpp"

namespace ota {

UpdateManager::UpdateManager(std::string initial_version) {
    slot_a_.version = std::move(initial_version);
    active_slot_ = 'A';
    message_ = "Up to date";
}

const std::string& UpdateManager::activeVersion() const { return active().version; }

bool UpdateManager::busy() const {
    switch (state_) {
    case State::Downloading:
    case State::Verifying:
    case State::Swapping:
    case State::HealthCheck:
        return true;
    default:
        return false;
    }
}

void UpdateManager::enter(State s) {
    state_ = s;
    elapsed_ = 0.0;
}

void UpdateManager::fail(const std::string& reason) {
    // The inactive slot may hold a half-written image; discard it. The active
    // slot is never touched mid-update, so we simply remain on it.
    inactive().version.clear();
    state_ = State::Failed;
    message_ = reason + " - rolled back to " + active().version;
}

void UpdateManager::requestUpdate(const std::string& target_version) {
    if (busy()) {
        return;
    }
    target_ = target_version;
    progress_ = 0.0;
    message_ = "Downloading " + target_;
    enter(State::Downloading);
}

void UpdateManager::reset() {
    state_ = State::Idle;
    progress_ = 0.0;
    elapsed_ = 0.0;
    message_ = "Up to date";
}

void UpdateManager::step(double dt) {
    if (dt <= 0.0) {
        return;
    }

    switch (state_) {
    case State::Downloading:
        progress_ += kDownloadRate * dt;
        // A simulated download error trips partway through the transfer.
        if (download_fail_ && progress_ >= 0.6) {
            fail("Download failed");
        } else if (progress_ >= 1.0) {
            progress_ = 1.0;
            message_ = "Verifying image";
            enter(State::Verifying);
        }
        break;

    case State::Verifying:
        elapsed_ += dt;
        if (elapsed_ >= kPhaseSeconds) {
            if (checksum_fail_) {
                fail("Checksum mismatch");
            } else {
                message_ = "Writing to inactive slot";
                enter(State::Swapping);
            }
        }
        break;

    case State::Swapping:
        elapsed_ += dt;
        if (elapsed_ >= kPhaseSeconds) {
            // Stage the new image on the inactive slot. The active slot still
            // runs the old version until the health check commits the swap.
            inactive().version = target_;
            message_ = "Running health check";
            enter(State::HealthCheck);
        }
        break;

    case State::HealthCheck:
        elapsed_ += dt;
        if (elapsed_ >= kPhaseSeconds) {
            if (health_fail_) {
                fail("Health check failed");
            } else {
                // Commit: the freshly written slot becomes active.
                active_slot_ = (active_slot_ == 'A') ? 'B' : 'A';
                state_ = State::Active;
                message_ = "Updated to " + target_;
            }
        }
        break;

    case State::Idle:
    case State::Active:
    case State::Failed:
        break;
    }
}

const char* toString(State s) {
    switch (s) {
    case State::Idle:
        return "Idle";
    case State::Downloading:
        return "Downloading";
    case State::Verifying:
        return "Verifying";
    case State::Swapping:
        return "Swapping";
    case State::HealthCheck:
        return "HealthCheck";
    case State::Active:
        return "Active";
    case State::Failed:
        return "Failed";
    }
    return "Unknown";
}

} // namespace ota
