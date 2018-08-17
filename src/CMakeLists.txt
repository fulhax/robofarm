cmake_minimum_required(VERSION 3.11)
project(robofarm)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(DevIL REQUIRED)

set(CMAKE_C_STANDARD 11)

add_executable(${CMAKE_PROJECT_NAME} main.c)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${IL_INCLUDE_DIR}
    )

message(STATUS ${OPENGL_LIBRARIES})
message(STATUS ${OPENGL_gl_LIBRARY})
target_link_libraries(${CMAKE_PROJECT_NAME}
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${IL_LIBRARIES}
    ${ILU_LIBRARIES}
    m
    robots
    nuklear
    )

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-Werror")
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-Wall")
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-Wpedantic")
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-Wno-missing-braces")
install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION bin)

add_subdirectory(editor)
add_subdirectory(libs)
