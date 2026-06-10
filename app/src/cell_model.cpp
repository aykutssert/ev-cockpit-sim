#include "cell_model.hpp"

#include <set>

int CellModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(rows_.size());
}

QVariant CellModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(rows_.size())) {
        return {};
    }
    const Row& r = rows_[static_cast<size_t>(index.row())];
    switch (role) {
    case SocRole:
        return r.soc;
    case TemperatureRole:
        return r.temperature;
    case FaultedRole:
        return r.faulted;
    default:
        return {};
    }
}

QHash<int, QByteArray> CellModel::roleNames() const {
    return {
        {SocRole, "soc"},
        {TemperatureRole, "temperature"},
        {FaultedRole, "faulted"},
    };
}

void CellModel::refresh(const bms::BatteryPack& pack) {
    const int n = pack.cellCount();

    // Which cells are currently flagged by a per-cell fault.
    std::set<int> faulted;
    for (const auto& f : pack.faults()) {
        if (f.cell_index >= 0) {
            faulted.insert(f.cell_index);
        }
    }

    if (static_cast<int>(rows_.size()) != n) {
        beginResetModel();
        rows_.assign(static_cast<size_t>(n), Row{});
        endResetModel();
    }

    for (int i = 0; i < n; ++i) {
        const bms::CellState& c = pack.cell(i);
        Row& r = rows_[static_cast<size_t>(i)];
        r.soc = c.soc;
        r.temperature = c.temperature;
        r.faulted = faulted.count(i) > 0;
    }
    if (n > 0) {
        emit dataChanged(index(0), index(n - 1), {SocRole, TemperatureRole, FaultedRole});
    }
}
