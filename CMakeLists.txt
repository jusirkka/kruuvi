cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(kruuvi LANGUAGES CXX)


#
# dependencies
#

set(QT_MIN_VERSION "5.15")

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
  DBus
  Sql
  Quick
)

set(KF5_MIN_VERSION "5.96")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
  BluezQt
  CoreAddons
  Plasma
)


#
# targets: plasma_engine_ruuvi_monitor
#
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
kcoreaddons_add_plugin(plasma_engine_ruuvi_monitor
  SOURCES
    src/ruuvimonitor.cpp
  INSTALL_NAMESPACE
    plasma/dataengine
)

set_target_properties(plasma_engine_ruuvi_monitor
  PROPERTIES
    AUTOMOC ON
)


target_include_directories(plasma_engine_ruuvi_monitor
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(plasma_engine_ruuvi_monitor
  PRIVATE
    cxx_std_17
)

target_link_libraries(plasma_engine_ruuvi_monitor
  PRIVATE
    Qt5::DBus
    KF5::BluezQt
    KF5::Plasma
)

#
# targets: plasmoid_plugin_db_reader
#

add_library(plasmoid_plugin_db_reader SHARED)

set_target_properties(plasmoid_plugin_db_reader
  PROPERTIES
    AUTOMOC ON
)

target_sources(plasmoid_plugin_db_reader
  PRIVATE
    src/plasmoidplugin.cpp
    src/dbreader.cpp
)

target_include_directories(plasmoid_plugin_db_reader
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/kruuvilib/src
)

target_link_libraries(plasmoid_plugin_db_reader
  PRIVATE
    KRuuviLib
    Qt5::Quick
    KF5::Plasma
)

target_compile_features(plasmoid_plugin_db_reader
  PRIVATE
    cxx_std_17
)

#
# targets: readlog
#

add_executable(kruuvi_readlog)

set_target_properties(kruuvi_readlog
  PROPERTIES
    AUTOMOC ON
)

target_sources(kruuvi_readlog
  PRIVATE
    logreader/src/main.cpp
    logreader/src/ruuvireader.cpp
)


target_include_directories(kruuvi_readlog
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/kruuvilib/src
)

target_compile_features(kruuvi_readlog
  PRIVATE
    cxx_std_17
)

target_link_libraries(kruuvi_readlog
  PRIVATE
    KRuuviLib
    Qt5::DBus
    KF5::BluezQt
)

#
# subdirectories
#

add_subdirectory(kruuvilib)

#
# monitorapplet sources
#

add_custom_target(Qml SOURCES
  monitorapplet/contents/ui/main.qml
  monitorapplet/contents/ui/CompactRepresentation.qml
  monitorapplet/contents/ui/FullRepresentation.qml
  monitorapplet/contents/ui/MeteoDisplay.qml
  monitorapplet/contents/ui/MeteoCanvas.qml
  monitorapplet/contents/ui/DimensionalValue.qml
  monitorapplet/contents/config/config.qml
  monitorapplet/contents/config/ConfigGeneral.qml
  monitorapplet/contents/ui/utils.js
)

#
# installation
#

include(GNUInstallDirs)

# applet & friends
plasma_install_package(monitorapplet kvanttiapina.kruuvi.monitor)
install(TARGETS plasmoid_plugin_db_reader DESTINATION ${QML_INSTALL_DIR}/kvanttiapina/kruuvi/private)
install(FILES src/qmldir DESTINATION ${QML_INSTALL_DIR}/kvanttiapina/kruuvi/private)

# log reader
install(TARGETS kruuvi_readlog DESTINATION ${CMAKE_INSTALL_BINDIR})

# icons
install(FILES data/ruuvitag-48.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps
  RENAME ruuvitag.png)
install(FILES data/ruuvitag-64.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps
  RENAME ruuvitag.png)
install(FILES data/ruuvitag-128.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps
  RENAME ruuvitag.png)
install(FILES data/ruuvitag-256.png
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
  RENAME ruuvitag.png)
