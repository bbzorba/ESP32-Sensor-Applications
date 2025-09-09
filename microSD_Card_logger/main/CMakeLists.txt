set(srcs "sd_card_controller.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES "fatfs" "esp_driver_sdmmc" "esp_driver_spi"
                       WHOLE_ARCHIVE)
