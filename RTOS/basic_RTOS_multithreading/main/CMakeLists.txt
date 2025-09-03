idf_component_register(SRCS "app_main.cpp" "button_led.cpp" "bme688_sensor.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES bme68x esp_driver_gpio esp_driver_i2c driver)
