idf_component_register(
    SRCS "app.cpp" "timerManager.cpp"
    INCLUDE_DIRS "."
    REQUIRES activeObject button led wifi
)