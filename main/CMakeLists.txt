idf_component_register(SRCS "hello_world_main.cpp"
                       PRIV_REQUIRES spi_flash driver nvs_flash bt
                       INCLUDE_DIRS ".")
