add_library(KRuuviLib STATIC)

set_target_properties(KRuuviLib
  PROPERTIES
    AUTOMOC ON
)

target_sources(KRuuviLib
  PRIVATE
    src/sqlitedatabase.cpp
    src/measurementdatabase.cpp
)

target_include_directories(KRuuviLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(KRuuviLib
  PRIVATE
    PROJECT_NAME="${CMAKE_PROJECT_NAME}"
)

target_link_libraries(KRuuviLib
  PRIVATE
    Qt5::Sql
    Qt5::DBus
)

target_compile_features(KRuuviLib
  PRIVATE
    cxx_std_17
)

