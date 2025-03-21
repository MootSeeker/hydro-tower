set( SOURCES app.cpp )

idf_component_register( SRCS ${SOURCES}
                        INCLUDE_DIRS "."
                        REQUIRES freertos esp_common driver esp_event esp_wifi nvs_flash )