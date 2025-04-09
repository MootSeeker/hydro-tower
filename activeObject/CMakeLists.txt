idf_component_register(
    SRCS "src/activeObject.cpp"
         "src/eventBus.cpp"
         "src/events.cpp"
         "src/timer.cpp"
    INCLUDE_DIRS "inc"
    REQUIRES freertos
)
