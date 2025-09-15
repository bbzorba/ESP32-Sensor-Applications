idf_component_register(SRCS "mlx90614_read_temp.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver
                       PRIV_REQUIRES mlx90614_lib)
