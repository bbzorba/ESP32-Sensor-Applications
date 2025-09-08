idf_component_register(
    SRCS "read_bme688.cpp"
    INCLUDE_DIRS "."
    REQUIRES bme68x driver spiffs
    )
