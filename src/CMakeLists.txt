cmake_minimum_required(VERSION 3.11)

project(robofarm)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(DevIL REQUIRED)

set(COMMON_SRC
    "main.c"
    "matrix.c"
    "notify.c"
    "options.c"
    "renderfunc.c"
    "resources.c"
    "resources/image.c"
    "resources/shader.c"
    "vector.c"
    )

include_directories(
    .
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${IL_INCLUDE_DIR}
    )



set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)


add_executable(${CMAKE_PROJECT_NAME} ${COMMON_SRC} ${SHADER_DEPEND})

target_link_libraries(${CMAKE_PROJECT_NAME}
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${IL_LIBRARIES}
    ${ILU_LIBRARIES}
    m
    )

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC "GL_GLEXT_PROTOTYPES")
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-Werror")
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-Wall")
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-Wpedantic")
install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION bin)
