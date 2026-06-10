#pragma once

#include <QAbstractListModel>

#include "bms/battery_pack.hpp"

// Exposes the pack's per-cell state to QML as a list model so a Repeater/grid
// can draw one delegate per cell. Roles: soc (0..1), temperature (deg C),
// faulted (bool).
class CellModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        SocRole = Qt::UserRole + 1,
        TemperatureRole,
        FaultedRole,
    };

    explicit CellModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Pull the latest snapshot from the pack and notify the view. faulted_cells
    // marks which cells currently have a fault so the delegate can highlight.
    void refresh(const bms::BatteryPack& pack);

private:
    struct Row {
        double soc = 1.0;
        double temperature = 25.0;
        bool faulted = false;
    };
    std::vector<Row> rows_;
};
