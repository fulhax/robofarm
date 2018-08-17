#include "nuklearui.h"
#include "options.h"
#include "editorui.h"
#include "opengl.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "notify.h"

struct nk_colorf bg = {0};
extern GLFWwindow* window;
struct nk_context* ctx = 0;
struct ui_images
{
    struct nk_image buttontest;
} ui_images;

struct ui_tiles
{
    struct dirent** filenames;
    int numfiles;
} ui_tiles;

struct tile_type
{
    char name[64];
} tiletypes[256] = {0};

unsigned char numtiletypes = 0;

int pngfilter(const struct dirent* entry)
{
    char* ext = strrchr(entry->d_name, '.');

    if(ext)
    {
        if(strcmp(ext, ".png") == 0)
        {
            return 1;
        }
    }

    return 0;
}

void readtilefolder()
{
    if(ui_tiles.filenames != 0)
    {
        for(int i = 0; i < ui_tiles.numfiles; i++)
        {
            free(ui_tiles.filenames[i]);
        }

        free(ui_tiles.filenames);
        ui_tiles.filenames = 0;
        ui_tiles.numfiles = 0;
    }

    ui_tiles.numfiles = scandir("./data/tiles", &ui_tiles.filenames, pngfilter, alphasort);

    if(ui_tiles.numfiles == -1)
    {
        perror("scandir");
    }

    for(int i = 0; i < ui_tiles.numfiles; i++)
    {
        printf("%s\n", ui_tiles.filenames[i]->d_name);
    }
}

void folderchangecallback(const char* dir)
{
    readtilefolder();
}

void ui_init()
{
    ctx = nk_ui_init();
    watchFile("./data/tiles", folderchangecallback);
    readtilefolder();
    ui_images.buttontest = nk_ui_image("./external/nuklear/example/icon/tools.png");
}

void ui_destroy()
{
    nk_ui_destroy();
}

void ui_render()
{
    nk_ui_render();
}

enum
{
    NORMAL_STATE,
    EDIT_TYPE
};
int state = NORMAL_STATE;
void ui_style_disabled(char status)
{
    if(status)
    {
        nk_style_default(ctx);
    }
    else
    {
        ctx->style.button.normal = nk_style_item_color(nk_rgb(40, 40, 40));
        ctx->style.button.hover = nk_style_item_color(nk_rgb(40, 40, 40));
        ctx->style.button.active = nk_style_item_color(nk_rgb(40, 40, 40));
        ctx->style.button.border_color = nk_rgb(60, 60, 60);
        ctx->style.button.text_background = nk_rgb(60, 60, 60);
        ctx->style.button.text_normal = nk_rgb(60, 60, 60);
        ctx->style.button.text_hover = nk_rgb(60, 60, 60);
        ctx->style.button.text_active = nk_rgb(60, 60, 60);
    }
}
void ui_logic()
{
    nk_ui_update();
    ui_style_disabled(state == NORMAL_STATE);

    if(nk_begin(ctx, "main window", nk_rect(options.width - 250, 0, 250, options.height), NK_WINDOW_BORDER))
    {
        nk_layout_row_dynamic(ctx, 16, 1);

        if(nk_button_label(ctx, "new type"))
        {
            state = EDIT_TYPE;
        }

        nk_layout_row_dynamic(ctx, 200, 1);
        nk_style_push_color(ctx, &ctx->style.window.border_color, nk_rgba_f(0.5, 0, 0, 1));
        nk_style_push_float(ctx, &ctx->style.window.border, 20);
        nk_group_begin(ctx, "type list", NK_WINDOW_BACKGROUND | NK_WINDOW_BORDER);
        nk_layout_row_dynamic(ctx, 32, 1);

        for(int i = 0; i < numtiletypes; i++)
        {
            if(nk_button_image_label(ctx, ui_images.buttontest, tiletypes[i].name, NK_TEXT_ALIGN_RIGHT) && state == NORMAL_STATE)
            {
                fprintf(stdout, "button pressed type:%i\n", i);
            }
        }

        nk_group_end(ctx);
        nk_style_pop_color(ctx);
        nk_style_pop_float(ctx);
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

        nk_end(ctx);
    }

    ui_style_disabled(state == EDIT_TYPE);

    if(state == EDIT_TYPE)
    {
        if(nk_begin(ctx, "edit type", nk_rect(50, options.height / 2 - 100, 250, 200), NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE))
        {
            static char typename[64] = {0};
            nk_layout_row_dynamic(ctx, 32, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX | NK_EDIT_AUTO_SELECT, typename, 64, nk_filter_ascii);
            nk_layout_row_dynamic(ctx, 16, 2);

            if(nk_button_label(ctx, "canel"))
            {
                state = NORMAL_STATE;
                typename[0] = 0;
            }

            if(nk_button_label(ctx, "save"))
            {
                state = NORMAL_STATE;
                snprintf(tiletypes[numtiletypes].name, 64, "%s", typename);
                numtiletypes++;
                typename[0] = 0;
            }
        }

        nk_end(ctx);
    }
}
