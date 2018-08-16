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

void ui_logic()
{
    nk_ui_update();

    if(nk_begin(ctx, "", nk_rect(options.width - 250, 0, 250, options.height), NK_WINDOW_BORDER))
    {
        enum {EASY, HARD};
        static int op = EASY;
        static int property = 20;
        nk_layout_row_static(ctx, 30, 80, 1);

        if(nk_button_label(ctx, "button"))
        {
            fprintf(stdout, "button pressed\n");
        }

        nk_layout_row_static(ctx, 48, 48, 1);

        if(nk_button_image(ctx, ui_images.buttontest))
        {
            fprintf(stdout, "button pressed\n");
        }

        nk_layout_row_static(ctx, 32, 32, 6);

        for(int i = 0; i < ui_tiles.numfiles; i++)
        {
            if(nk_button_image(ctx, ui_images.buttontest))
            {
                fprintf(stdout, "button pressed\n");
            }
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
}
