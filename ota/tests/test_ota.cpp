// Lightweight assert-based tests for the OTA update state machine.
#include "ota/update_manager.hpp"

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

// Drive the machine to a terminal state (Active/Failed/Idle) or give up.
void runToTerminal(ota::UpdateManager& m) {
    for (int i = 0; i < 1000 && m.busy(); ++i) {
        m.step(0.25);
    }
}

void test_happy_path() {
    ota::UpdateManager m("1.4.0");
    check(m.activeSlot() == 'A', "starts on slot A");
    check(m.activeVersion() == "1.4.0", "starts on initial version");

    m.requestUpdate("1.5.0");
    check(m.state() == ota::State::Downloading, "request starts download");
    runToTerminal(m);

    check(m.state() == ota::State::Active, "happy path reaches Active");
    check(m.activeVersion() == "1.5.0", "active version is the new one");
    check(m.activeSlot() == 'B', "active slot flipped to B");
}

void test_download_failure() {
    ota::UpdateManager m("1.4.0");
    m.setDownloadShouldFail(true);
    m.requestUpdate("1.5.0");
    runToTerminal(m);

    check(m.state() == ota::State::Failed, "download failure ends in Failed");
    check(m.activeVersion() == "1.4.0", "download failure keeps old version");
    check(m.activeSlot() == 'A', "download failure does not flip slot");
}

void test_checksum_failure() {
    ota::UpdateManager m("1.4.0");
    m.setChecksumShouldFail(true);
    m.requestUpdate("1.5.0");
    runToTerminal(m);

    check(m.state() == ota::State::Failed, "checksum failure ends in Failed");
    check(m.activeVersion() == "1.4.0", "checksum failure keeps old version");
}

void test_health_failure_rolls_back() {
    ota::UpdateManager m("1.4.0");
    m.setHealthShouldFail(true);
    m.requestUpdate("1.5.0");
    runToTerminal(m);

    // The new image was written to the inactive slot but the health check
    // failed, so the active slot must remain the known-good previous version.
    check(m.state() == ota::State::Failed, "health failure ends in Failed");
    check(m.activeVersion() == "1.4.0", "health failure rolls back to old version");
    check(m.activeSlot() == 'A', "health failure keeps slot A active");
}

void test_ab_alternation() {
    ota::UpdateManager m("1.4.0");
    m.requestUpdate("1.5.0");
    runToTerminal(m);
    check(m.activeSlot() == 'B' && m.activeVersion() == "1.5.0", "first update -> slot B");

    m.requestUpdate("1.6.0");
    check(m.state() == ota::State::Downloading, "can start a new update from Active");
    runToTerminal(m);
    check(m.activeSlot() == 'A' && m.activeVersion() == "1.6.0", "second update -> back to slot A");
}

void test_no_restart_midflight() {
    ota::UpdateManager m("1.4.0");
    m.requestUpdate("1.5.0");
    m.step(0.25); // now mid-download
    check(m.busy(), "machine is busy mid-download");
    m.requestUpdate("9.9.9"); // should be ignored
    check(m.targetVersion() == "1.5.0", "request during an update is ignored");
}

void test_determinism() {
    auto run = []() {
        ota::UpdateManager m("2.0.0");
        m.requestUpdate("2.1.0");
        std::string trace;
        for (int i = 0; i < 50; ++i) {
            m.step(0.2);
            trace += ota::toString(m.state());
        }
        return trace;
    };
    check(run() == run(), "two identical runs produce the same state trace");
}

} // namespace

int main() {
    std::printf("running ota tests...\n");
    test_happy_path();
    test_download_failure();
    test_checksum_failure();
    test_health_failure_rolls_back();
    test_ab_alternation();
    test_no_restart_midflight();
    test_determinism();

    if (g_failures == 0) {
        std::printf("all ota tests passed\n");
        return 0;
    }
    std::printf("%d ota test(s) failed\n", g_failures);
    return 1;
}
