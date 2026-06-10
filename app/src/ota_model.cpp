#include "ota_model.hpp"

OtaModel::OtaModel(QObject* parent) : QObject(parent) {
    connect(&timer_, &QTimer::timeout, this, &OtaModel::tick);
    timer_.start(kTickMs);
}

QString OtaModel::stateText() const {
    switch (mgr_.state()) {
    case ota::State::Idle:
        return QStringLiteral("Idle");
    case ota::State::Downloading:
        return QStringLiteral("Downloading");
    case ota::State::Verifying:
        return QStringLiteral("Verifying");
    case ota::State::Swapping:
        return QStringLiteral("Writing slot");
    case ota::State::HealthCheck:
        return QStringLiteral("Health check");
    case ota::State::Active:
        return QStringLiteral("Up to date");
    case ota::State::Failed:
        return QStringLiteral("Failed");
    }
    return {};
}

int OtaModel::stepIndex() const {
    switch (mgr_.state()) {
    case ota::State::Downloading:
        return 0;
    case ota::State::Verifying:
        return 1;
    case ota::State::Swapping:
        return 2;
    case ota::State::HealthCheck:
        return 3;
    default:
        return -1;
    }
}

bool OtaModel::updateAvailable() const {
    return !mgr_.busy() && activeVersion() != available_;
}

void OtaModel::install() {
    mgr_.requestUpdate(available_.toStdString());
    emit changed();
}

void OtaModel::reset() {
    mgr_.reset();
    emit changed();
}

void OtaModel::setFailDownload(bool v) {
    mgr_.setDownloadShouldFail(v);
    emit changed();
}

void OtaModel::setFailChecksum(bool v) {
    mgr_.setChecksumShouldFail(v);
    emit changed();
}

void OtaModel::setFailHealth(bool v) {
    mgr_.setHealthShouldFail(v);
    emit changed();
}

void OtaModel::tick() {
    const ota::State before = mgr_.state();
    const double before_progress = mgr_.progress();
    mgr_.step(kSimDtSeconds);
    if (mgr_.state() != before || mgr_.progress() != before_progress) {
        emit changed();
    }
}
