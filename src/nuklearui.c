#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nuklearui.h"
#include "opengl.h"
#include "renderfunc.h"

typedef struct nk_context nk_context;
#define MAX_TEXT 256

struct ui_internals
{
    nk_context context;
    float scroll_x;
    float scroll_y;
    unsigned int text[MAX_TEXT];
    unsigned int text_length;
    double mouse_double_x;
    double mouse_double_y;
    double last_button_click;
    char double_click_down;
} ui = {0};

void nk_ui_mouse_button_callback(struct GLFWwindow* win, int button, int action, int mods)
{
    if(button != GLFW_MOUSE_BUTTON_LEFT)
    {
        return;
    }

    if(action == GLFW_PRESS)
    {
        double deltatime = glfwGetTime() - ui.last_button_click;

        if(deltatime > 0.02 && deltatime < 0.2)
        {
            ui.double_click_down = 1;
            glfwGetCursorPos(win, &ui.mouse_double_x, &ui.mouse_double_y);
        }

        ui.last_button_click = glfwGetTime();
    }
    else
    {
        ui.double_click_down = 0;
    }
}

void nk_ui_char_callback(struct GLFWwindow* win, unsigned int codepoint)
{
    if(ui.text_length < MAX_TEXT)
    {
        ui.text[ui.text_length++] = codepoint;
    }
}

void nk_ui_scroll_callback(struct GLFWwindow* win, double xoffset, double yoffset)
{
    ui.scroll_x += xoffset;
    ui.scroll_y += yoffset;
}

nk_context* nk_ui_init()
{
    if(!nk_init_default(&ui.context, 0))
    {
        fprintf(stderr, "failed to create nuklear ui\n");
        exit(1);
    }

    glfwSetScrollCallback(window, nk_ui_scroll_callback);
    glfwSetCharCallback(window, nk_ui_char_callback);
    glfwSetMouseButtonCallback(window, nk_ui_mouse_button_callback);
    return &ui.context;
}

void nk_ui_destroy()
{
    nk_free(&ui.context);
}

void nk_ui_render()
{
}

void nk_ui_update()
{
}
