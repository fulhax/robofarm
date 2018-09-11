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

typedef struct tile_image
{
    struct nk_image img;
    char* filename;
} tile_image;

struct ui_images
{
    struct nk_image buttontest;
    struct nk_image* tiles;
    unsigned int numtiles;
} ui_images = {0};

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
struct ui_style
{
    struct nk_style_button button_selected;
} ui_style;

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
    ui_style.button_selected = ctx->style.button;
    ui_style.button_selected.border_color.r = 0xfb;
    ui_style.button_selected.border_color.g = 0x89;
    ui_style.button_selected.border_color.b = 0x02;
    ui_style.button_selected.border_color.a = 0xff;
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

struct nk_style_button* ui_style_selected_button(char test)
{
    if(test)
    {
        return &ui_style.button_selected;
    }
    else
    {
        return &ctx->style.button;
    }
}

void ui_logic()
{
    nk_ui_update();
    ui_style_disabled(state == NORMAL_STATE);

    if(nk_begin(ctx, "main window", nk_rect(0, 0, options.width, options.height), NK_WINDOW_BORDER))
    {
        nk_layout_row_dynamic(ctx, 16, 2);
        nk_label(ctx, "Types", NK_TEXT_CENTERED);
        nk_label(ctx, "Selected", NK_TEXT_CENTERED);

        if(nk_button_label(ctx, "new type"))
        {
            state = EDIT_TYPE;
        }

        nk_layout_row_dynamic(ctx, 200, 2);
        nk_group_begin(ctx, "type list", NK_WINDOW_BACKGROUND | NK_WINDOW_BORDER);
        {
            nk_layout_row_dynamic(ctx, 32, 1);
            static int selectedbutton = 0;

            for(int i = 0; i < numtiletypes; i++)
            {
                if(nk_button_image_label_styled(ctx, ui_style_selected_button(i == selectedbutton), ui_images.buttontest, tiletypes[i].name, NK_TEXT_ALIGN_RIGHT) && state == NORMAL_STATE)
                {
                    selectedbutton = i;
                    fprintf(stdout, "button pressed type:%i\n", i);
                }
            }
        }
        nk_group_end(ctx);
        nk_group_begin(ctx, "type list", NK_WINDOW_BACKGROUND | NK_WINDOW_BORDER);
        nk_group_end(ctx);
        nk_layout_row_dynamic(ctx, 16, 1);
        nk_label(ctx, "Images", NK_TEXT_CENTERED);
        int height = options.height - 290;

        if(height < 250)
        {
            height = 250;
        }

        nk_layout_row_dynamic(ctx, height, 1);
        nk_group_begin(ctx, "tilelist", NK_WINDOW_BACKGROUND | NK_WINDOW_BORDER);
        {
            struct nk_rect r = nk_window_get_content_region(ctx);
            int cols = (int)r.w / (32 + ctx->style.button.padding.x * 2);
            nk_layout_row_static(ctx, 32, 32, cols);

            for(int i = 0; i < ui_tiles.numfiles; i++)
            {
                nk_image(ctx, ui_images.buttontest);
            }
        }
        nk_group_end(ctx);
        nk_end(ctx);
    }

    ui_style_disabled(state == EDIT_TYPE);

    if(state == EDIT_TYPE)
    {
        int height = options.height / 2 - 100;

        if(height > 50)
        {
            height = 50;
        }

        if(nk_begin(ctx, "edit type", nk_rect(50, height, 250, 200), NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE))
        {
            static char typename[64] = {0};
            nk_layout_row_dynamic(ctx, 32, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_SIG_ENTER | NK_TEXT_EDIT_SINGLE_LINE | NK_EDIT_BOX | NK_EDIT_AUTO_SELECT, typename, 64, nk_filter_ascii);
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
        nk_window_set_focus(ctx, "edit type");
    }
}
