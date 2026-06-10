find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(ev_cockpit_sim
  src/main.cpp
  src/vehicle_model.cpp
  src/cell_model.cpp
)

qt_add_qml_module(ev_cockpit_sim
  URI EvCockpit
  VERSION 1.0
  QML_FILES
    qml/Main.qml
    qml/SocGauge.qml
    qml/StatTile.qml
    qml/BatteryGrid.qml
)

target_link_libraries(ev_cockpit_sim PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  Qt6::QuickControls2
  bms
)
