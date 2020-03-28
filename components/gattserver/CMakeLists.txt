idf_component_register(
    SRCS "GattServer.cpp" "Service.cpp"  "UUID.cpp" "Characteristic.cpp"
    INCLUDE_DIRS "include"
    REQUIRES bt
)