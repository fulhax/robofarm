#ifndef __NUKLEARUI_H__
#define __NUKLEARUI_H__

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_UINT_DRAW_INDEX

#include <stdarg.h>
#include <nuklear.h>

struct nk_context* nk_ui_init();
void nk_ui_destroy();
void nk_ui_render(int width,int height);
void nk_ui_update();
struct nk_image nk_ui_image(char* filename);

#endif //__NUKLEARUI_H__
