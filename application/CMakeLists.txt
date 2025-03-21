set( SOURCES app.cpp )

idf_component_register(
    SRCS "app.cpp"
    INCLUDE_DIRS "."
    REQUIRES freertos esp_common driver esp_event esp_wifi nvs_flash sensors display mqtt
)