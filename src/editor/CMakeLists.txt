include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../external/nuklear
    ${CMAKE_SOURCE_DIR}/libs
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    )

add_executable(tileeditor
    editor.c
    editorui.c
    )

target_link_libraries(tileeditor
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${IL_LIBRARIES}
    ${ILU_LIBRARIES}
    m
    robots
    nuklear
    )

target_compile_options(tileeditor PUBLIC "-Werror")
target_compile_options(tileeditor PUBLIC "-Wall")
target_compile_options(tileeditor PUBLIC "-Wpedantic")
install(TARGETS tileeditor RUNTIME DESTINATION bin)
