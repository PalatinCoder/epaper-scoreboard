idf_component_register(
    SRCS "GattServer.cpp" "Service.cpp"  "UUID.cpp" "Characteristic.cpp" "Descriptor.cpp" "Special/BatteryLevel.cpp"
    INCLUDE_DIRS "include"
    REQUIRES bt
)