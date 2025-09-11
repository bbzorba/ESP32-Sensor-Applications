idf_component_register(SRCS "sdcard_lib.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "fatfs" "esp_driver_sdmmc" "esp_driver_spi"
                    WHOLE_ARCHIVE)
