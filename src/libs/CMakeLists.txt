add_library(nuklear STATIC "nuklear.c")

target_include_directories(nuklear SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/../external/nuklear
    )
target_include_directories(nuklear PRIVATE
    ${CMAKE_SOURCE_DIR}
    )


add_library(robots STATIC
    "${CMAKE_SOURCE_DIR}/matrix.c"
    "${CMAKE_SOURCE_DIR}/notify.c"
    "${CMAKE_SOURCE_DIR}/options.c"
    "${CMAKE_SOURCE_DIR}/renderfunc.c"
    "${CMAKE_SOURCE_DIR}/resources.c"
    "${CMAKE_SOURCE_DIR}/resources/image.c"
    "${CMAKE_SOURCE_DIR}/resources/shader.c"
    "${CMAKE_SOURCE_DIR}/vector.c"
    "${CMAKE_SOURCE_DIR}/nuklearui.c"
    )

target_compile_options(robots PUBLIC "-Werror")
target_compile_options(robots PUBLIC "-Wall")
target_compile_options(robots PUBLIC "-Wpedantic")
target_include_directories(robots SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/../external/nuklear
    )
target_include_directories(robots PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${IL_INCLUDE_DIR}
    )
