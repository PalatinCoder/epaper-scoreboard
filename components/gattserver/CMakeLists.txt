idf_component_register(
    SRCS "GattServer.cpp" "Service.cpp"  "UUID.cpp"
    INCLUDE_DIRS "include"
    REQUIRES bt
)