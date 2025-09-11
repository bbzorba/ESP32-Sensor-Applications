set(srcs "environmental_data_recorder_app.cpp")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "."
                       REQUIRES "fatfs" "esp_driver_sdmmc" "esp_driver_spi" "driver" "spiffs"
                       PRIV_REQUIRES bme688_lib sdcard_lib
                       WHOLE_ARCHIVE)
