set(srcs "environmental_data_recorder_app.cpp")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "."
                       REQUIRES "fatfs" "sdmmc" "driver" "spiffs"
                       PRIV_REQUIRES bme688_lib sdcard_lib)
