# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Uncomment once you drop a SquareLine Studio export into squareline_ui/
# (see squareline_ui section of README.md for the component CMakeLists.txt)
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/squareline_ui")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32-gui)
