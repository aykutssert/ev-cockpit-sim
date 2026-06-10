#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include "ota/update_manager.hpp"

// Qt wrapper around the OTA update state machine. Owns a timer that advances
// the flow and republishes its state to QML. Decisions and transitions live in
// the Qt-free ota::UpdateManager; this class is just the bridge.
class OtaModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString stateText READ stateText NOTIFY changed)
    Q_PROPERTY(int stepIndex READ stepIndex NOTIFY changed)
    Q_PROPERTY(double progress READ progress NOTIFY changed)
    Q_PROPERTY(QString activeVersion READ activeVersion NOTIFY changed)
    Q_PROPERTY(QString activeSlot READ activeSlot NOTIFY changed)
    Q_PROPERTY(QString availableVersion READ availableVersion CONSTANT)
    Q_PROPERTY(QString message READ message NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool failed READ failed NOTIFY changed)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY changed)

public:
    explicit OtaModel(QObject* parent = nullptr);

    QString stateText() const;
    int stepIndex() const; // 0..3 for Download/Verify/Swap/Health, -1 otherwise
    double progress() const { return mgr_.progress(); }
    QString activeVersion() const { return QString::fromStdString(mgr_.activeVersion()); }
    QString activeSlot() const { return QString(QChar(mgr_.activeSlot())); }
    QString availableVersion() const { return available_; }
    QString message() const { return QString::fromStdString(mgr_.message()); }
    bool busy() const { return mgr_.busy(); }
    bool failed() const { return mgr_.state() == ota::State::Failed; }
    bool updateAvailable() const;

    Q_INVOKABLE void install();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void setFailDownload(bool v);
    Q_INVOKABLE void setFailChecksum(bool v);
    Q_INVOKABLE void setFailHealth(bool v);

signals:
    void changed();

private:
    void tick();

    static constexpr double kSimDtSeconds = 0.3;
    static constexpr int kTickMs = 100;

    ota::UpdateManager mgr_;
    QTimer timer_;
    QString available_ = QStringLiteral("1.5.0");
};
