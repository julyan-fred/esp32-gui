idf_component_register(SRCS "main.c" "display.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_lcd squareline_ui)
