# Auto-collects whatever SquareLine Studio exports into this folder.
# Works when the folder is empty (registers as an interface library), so the
# project always builds - drop the export in and it starts compiling.

file(GLOB_RECURSE UI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

set(UI_INCLUDE_DIRS ".")
file(GLOB UI_CHILDREN LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(child ${UI_CHILDREN})
    if(IS_DIRECTORY ${child})
        list(APPEND UI_INCLUDE_DIRS ${child})
    endif()
endforeach()

idf_component_register(SRCS ${UI_SOURCES}
                       INCLUDE_DIRS ${UI_INCLUDE_DIRS})
