#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#include <string.h>
#include "nuklearui.h"

typedef struct nk_context nk_context;
nk_context ui_context = {0};

nk_context* nk_ui_init()
{
    if(!nk_init_default(&ui_context, 0))
    {
        fprintf(stderr, "failed to create nuklear ui\n");
        exit(1);
    }

    return &ui_context;
}

void nk_ui_destroy()
{
    nk_free(&ui_context);
}

void nk_ui_render()
{
}

void nk_ui_update()
{
}
