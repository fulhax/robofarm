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
    window = glfwCreateWindow(options.width, options.height, "robofarm", NULL, NULL);

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

    while(!glfwWindowShouldClose(window) && !should_quit)
    {
        handleMouse();
        handleKeys();
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
        watchChanges();
        updateTime();
    }

    cleanupImages();
    cleanupShaders();
    destroyFileWatcher();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
