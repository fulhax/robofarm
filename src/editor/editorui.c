#include "nuklearui.h"
#include "opengl.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

extern GLFWwindow* window;
struct nk_context* ctx;

void ui_init()
{
    /*ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);*/
}

void ui_destroy()
{
}

void ui_render()
{
}

void ui_logic()
{
}
