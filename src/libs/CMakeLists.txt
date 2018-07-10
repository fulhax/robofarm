include_directories(
    ${CMAKE_SOURCE_DIR}/../external/nuklear
    )

add_library(nuklear STATIC "nuklear.c")
