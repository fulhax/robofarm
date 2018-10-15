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
#include "editorui.h"
#include <dlfcn.h>
#include <signal.h>
#include <setjmp.h>
#include <execinfo.h>

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

void* libedituihandle = 0;
void (*uidestroy)() = 0;
void (*uiinit)(GLFWwindow* w) = 0;
void (*uilogic)(int width, int hegiht) = 0;
void (*uirender)(int width, int height) = 0;

char needsreload = 0;
sigjmp_buf before = {0};

void print_trace(void)
{
    void* array[50];
    size_t size;
    char** strings;
    size_t i;
    size = backtrace(array, 50);
    strings = backtrace_symbols(array, size);
    fprintf(stderr, "Obtained %zd stack frames.\n", size);

    for(i = 0; i < size; i++)
    {
        fprintf(stderr, "%s\n", strings[i]);
    }

    free(strings);
}

static void sighandler(int sig, siginfo_t* info, void* ucontext)
{
    print_trace();
    needsreload = 1;
    longjmp(before, 1);
}

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
        if(!needsreload)
        {
            printf("%i fps frametime:%f\n", framecounter, deltaTime);
        }

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

void loadlibeditorui()
{
    if(libedituihandle != 0)
    {
        dlclose(libedituihandle);
        libedituihandle = 0;
    }

    libedituihandle = dlopen("./build/editor/libeditorui.so", RTLD_LAZY);

    if(libedituihandle == 0)
    {
        printf("error loading libeditorui.so\n%s\n", dlerror());
        system("ls ./build/editor/libeditorui.so");
        exit(1);
    }

    *(void**)(&uidestroy) = dlsym(libedituihandle, "ui_destroy");
    *(void**)(&uiinit) = dlsym(libedituihandle, "ui_init");
    *(void**)(&uilogic) = dlsym(libedituihandle, "ui_logic");
    *(void**)(&uirender) = dlsym(libedituihandle, "ui_render");
    printf("loaded lib\n");
}

void reloadeditorui(const char* filename)
{
    system("make -C build");
    uidestroy();
    loadlibeditorui();
    uiinit(window);
    needsreload = 0;
}

int main(int argc, char* argv[])
{
    loadlibeditorui();

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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
    watchFile("./src/editor/editorui.c", reloadeditorui);
    ilInit();
    iluInit();
    initImages();
    struct sigaction sa = {0};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NODEFER;
    sa.sa_sigaction = sighandler;
    sigaction(SIGSEGV, &sa, 0);
    uiinit(window);

    while(!glfwWindowShouldClose(window) && !should_quit)
    {
        handleMouse();
        handleKeys();

        if(!needsreload)
        {
            if(setjmp(before) == 0)
            {
                uilogic(options.width, options.height);
            }
        }

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(!needsreload)
        {
            if(setjmp(before) == 0)
            {
                uirender(options.width, options.height);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        watchChanges();
        updateTime();
    }

    if(!needsreload)
    {
        if(setjmp(before) == 0)
        {
            uidestroy();
        }
    }

    sa.sa_sigaction = 0;
    sa.sa_handler = SIG_DFL;
    sigaction(SIGSEGV, &sa, 0);
    cleanupImages();
    cleanupShaders();
    destroyFileWatcher();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
