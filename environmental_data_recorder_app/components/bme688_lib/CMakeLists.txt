idf_component_register(SRCS "bme688_lib.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES bme68x driver spiffs esp_timer freertos driver esp_rom
                    WHOLE_ARCHIVE)
