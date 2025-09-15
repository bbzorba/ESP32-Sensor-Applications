idf_component_register(SRCS "lora_receiver.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES "driver"
                    PRIV_REQUIRES qwiicrf_lib)
