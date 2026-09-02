cmake_minimum_required(VERSION 3.16)
project(esp32_gui_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Panel geometry the sim window mimics. Override at configure time:
#   cmake -B build -DSIM_HOR_RES=800 -DSIM_VER_RES=480 -DSIM_ZOOM=1
set(SIM_HOR_RES 240 CACHE STRING "Simulated panel width in px")
set(SIM_VER_RES 320 CACHE STRING "Simulated panel height in px")
set(SIM_ZOOM    2   CACHE STRING "Window scale factor")

# We only want the LVGL library itself, not its demo/example suites.
set(CONFIG_LV_BUILD_DEMOS    OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# LVGL picks up lv_conf.h from CMAKE_SOURCE_DIR (this directory) automatically.
add_subdirectory(lvgl)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

# The SquareLine export - the exact same .c files the ESP32 build compiles.
# Empty today; picked up automatically once you drop an export in.
set(UI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../squareline_ui)
file(GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS ${UI_DIR}/*.c)

set(UI_INCLUDE_DIRS ${UI_DIR})
file(GLOB UI_CHILDREN LIST_DIRECTORIES true CONFIGURE_DEPENDS ${UI_DIR}/*)
foreach(child ${UI_CHILDREN})
    if(IS_DIRECTORY ${child})
        list(APPEND UI_INCLUDE_DIRS ${child})
    endif()
endforeach()

add_executable(sim main.c ${UI_SOURCES})

target_compile_definitions(sim PRIVATE
    SIM_HOR_RES=${SIM_HOR_RES}
    SIM_VER_RES=${SIM_VER_RES}
    SIM_ZOOM=${SIM_ZOOM})

target_include_directories(sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${UI_INCLUDE_DIRS})
target_link_libraries(sim PRIVATE lvgl PkgConfig::SDL2)
