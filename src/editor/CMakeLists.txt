include_directories( SYSTEM
    ${CMAKE_SOURCE_DIR}/../external/nuklear
    )
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    )

add_executable(tileeditor
    editor.c
    )
add_library(editorui SHARED editorui.c)
#target_link_libraries(editorui nuklear)

target_link_libraries(tileeditor
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${IL_LIBRARIES}
    ${ILU_LIBRARIES}
    m
    -Wl,--whole-archive
    robots
    nuklear
    -Wl,--no-whole-archive
    dl
    -rdynamic
    )

target_compile_options(tileeditor PUBLIC "-Werror")
target_compile_options(tileeditor PUBLIC "-Wall")
target_compile_options(tileeditor PUBLIC "-Wpedantic")
target_compile_options(tileeditor PUBLIC "-Wno-missing-braces")
target_compile_options(editorui PUBLIC "-fPIC")
target_compile_options(editorui PUBLIC "-shared")
install(TARGETS tileeditor RUNTIME DESTINATION bin)
