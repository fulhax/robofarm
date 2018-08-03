#include "opengl.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <stdio.h>
#include "options.h"
#include "resources.h"
#include <unistd.h>
#include "notify.h"
#include <stdlib.h>
#include "defines.h"
#include "renderfunc.h"
#include "nuklearui.h"

GLFWwindow* window = 0;
int should_quit = 0;
double deltaTime = 0;
double currenttime = 0;
double lasttime = 0;
double xpos = 0;
double ypos = 0;
char mousebuttons[8] = {0};
char mouseinside = 0;
double lastxpos = 0;
double lastypos = 0;
char lastmousebuttons[8] = {0};
char lastmouseinside = 0;

char keyboard[GLFW_KEY_LAST + 1] = {0};
char keyboardlast[GLFW_KEY_LAST + 1] = {0};


void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    options.width = width;
    options.height = height;
    resizeTargets();
}
void key_handler_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        should_quit = 1;
        printf("abort abort!\n");
    }
}
void updateTime() // call once per frame
{
    currenttime = glfwGetTime();
    static double lastdelta = 0;
    deltaTime = currenttime - lastdelta;
    lastdelta = currenttime;
    static int framecounter = 0;
    framecounter++;

    if(currenttime - lasttime > 1.0)
    {
        printf("%i fps frametime:%f\n", framecounter, deltaTime);
        framecounter = 0;
        lasttime = currenttime;
    }
}

void handleMouse()
{
    lastxpos = xpos;
    lastypos = ypos;
    lastmouseinside = mouseinside;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouseinside = 1;

    if(xpos < 0 || xpos >= options.width)
    {
        mouseinside = 0;
    }

    if(ypos < 0 || ypos >= options.height)
    {
        mouseinside = 0;
    }

    for(int i = 0; i < 8; i++)
    {
        lastmousebuttons[i] = mousebuttons[i];

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1 + i) == GLFW_PRESS)
        {
            mousebuttons[i] = 1;
        }
        else
        {
            mousebuttons[i] = 0;
        }
    }
}

void handleKeys()
{
    for(int i = GLFW_KEY_SPACE; i < GLFW_KEY_LAST + 1; i++)
    {
        keyboardlast[i] = keyboard[i];
        keyboard[i] = glfwGetKey(window, i) == GLFW_PRESS;
    }
}

int main(int argc, char* argv[])
{
    if(!handle_options(argc, argv))
    {
        return 0;
    }

    if(!glfwInit())
    {
        fprintf(stderr, "Error: initing glfw\n");
        return 1;
    }

    if(!initFileWatcher())
    {
        fprintf(stderr, "Error: initing filewatcher\n");
        return 1;
    }

    glfwSetErrorCallback(error_callback);
    window = glfwCreateWindow(options.width, options.height, "robofarm - editor", NULL, NULL);

    if(!window)
    {
        fprintf(stderr, "Error: creating the window\n");
        return 1;
    }

    if(!(options.x == INT_MIN || options.y == INT_MIN))
    {
        printf("%i,%i\n", options.x, options.y);
        glfwSetWindowPos(window, options.x, options.y);
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_handler_callback);
    glfwSwapInterval(0);
    glfwSwapBuffers(window);
    ilInit();
    iluInit();
    initImages();
    struct nk_context* ctx = nk_ui_init();
    struct nk_colorf bg = {0};

    while(!glfwWindowShouldClose(window) && !should_quit)
    {
        handleMouse();
        handleKeys();
        nk_ui_update();

        /* GUI */
        if(nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
                    NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                    NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;
            nk_layout_row_static(ctx, 30, 80, 1);

            if(nk_button_label(ctx, "button"))
            {
                fprintf(stdout, "button pressed\n");
            }

            nk_layout_row_dynamic(ctx, 30, 2);

            if(nk_option_label(ctx, "easy", op == EASY))
            {
                op = EASY;
            }

            if(nk_option_label(ctx, "hard", op == HARD))
            {
                op = HARD;
            }

            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);

            if(nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx), 400)))
            {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
                nk_combo_end(ctx);
            }
        }

        nk_end(ctx);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        nk_ui_render();
        glfwSwapBuffers(window);
        glfwPollEvents();
        watchChanges();
        updateTime();
    }

    nk_ui_destroy();
    cleanupImages();
    cleanupShaders();
    destroyFileWatcher();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
