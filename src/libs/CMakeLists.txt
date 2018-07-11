include_directories(
    ${CMAKE_SOURCE_DIR}/../external/nuklear
    ${CMAKE_SOURCE_DIR}/../external/nuklear/demo/glfw_opengl4
    )

add_library(nuklear STATIC "nuklear.c")
