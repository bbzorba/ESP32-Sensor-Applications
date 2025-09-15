idf_component_register(SRCS "sdcard_lib.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "fatfs" "sdmmc" "driver")
