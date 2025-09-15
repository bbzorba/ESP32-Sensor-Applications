idf_component_register(SRCS "mlx90614_lib.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_timer
                    PRIV_REQUIRES log)
