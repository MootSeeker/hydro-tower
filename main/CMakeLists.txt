idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES application esp_event nvs_flash driver
)
