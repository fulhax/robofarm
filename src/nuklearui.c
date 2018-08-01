#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nuklearui.h"
#include "opengl.h"
#include "renderfunc.h"
#include "resources.h"
#include <unistd.h>

typedef struct nk_context nk_context;
typedef struct nk_font_atlas nk_font_atlas;
typedef struct nk_buffer nk_buffer;
typedef struct nk_draw_command nk_draw_command;

#define MAX_TEXT 256
#define BUFFER_LEN 8192

typedef struct ui_vertex
{
    float pos[3];
    float uv[2];
    char color[4];
} ui_vertex;

struct ui_internals
{
    nk_context context;
    nk_font_atlas font_atlas;
    struct nk_vec2 scroll;
    unsigned int text[MAX_TEXT];
    unsigned int text_length;
    double mouse_double_x;
    double mouse_double_y;
    double last_button_click;
    char double_click_down;
    nk_buffer cmds;
    ui_vertex vertexbuffer[BUFFER_LEN];
    unsigned short indexbuffer[BUFFER_LEN];
    int shader;
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
    ui.scroll.x += xoffset;
    ui.scroll.y += yoffset;
}

void nk_ui_clipboard_paste(nk_handle usr, struct nk_text_edit* edit)
{
    const char* text = glfwGetClipboardString(window);

    if(text)
    {
        nk_textedit_paste(edit, text, nk_strlen(text));
    }
}

void nk_ui_clipboard_copy(nk_handle usr, const char* text, int len)
{
    char* str = 0;

    if(!len)
    {
        return;
    }

    str = (char*)malloc((size_t)len + 1);

    if(!str)
    {
        return;
    }

    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    glfwSetClipboardString(window, str);
    free(str);
}

void nk_ui_fontsetup()
{
    nk_font_atlas_init_default(&ui.font_atlas);
    int w;
    int h;
    nk_font_atlas_bake(&ui.font_atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    printf("font size:%i %i\n", w, h);
    nk_style_set_font(&ui.context, &ui.font_atlas.default_font->handle);
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
    ui.context.clip.paste = nk_ui_clipboard_paste;
    ui.context.clip.copy = nk_ui_clipboard_copy;
    ui.context.clip.userdata = nk_handle_ptr(0);
    nk_ui_fontsetup();
    memset(ui.indexbuffer, 0, sizeof(unsigned short) * BUFFER_LEN);
    memset(ui.vertexbuffer, 0, sizeof(ui_vertex) * BUFFER_LEN);
    ui.shader = loadShader("shaders/ui/ui.vert", "shaders/ui/ui.frag", 0, 0, 0);
    return &ui.context;
}

void nk_ui_destroy()
{
    nk_free(&ui.context);
}

void nk_ui_render()
{
    nk_buffer vbuf = {0};
    nk_buffer ibuf = {0};
    struct nk_convert_config cfg = {0};
    static const struct nk_draw_vertex_layout_element vertex_layout[] =
    {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(ui_vertex, pos)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(ui_vertex, uv)},
        {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(ui_vertex, color)},
        {NK_VERTEX_LAYOUT_END}
    };
    cfg.vertex_layout = vertex_layout;
    cfg.vertex_size = sizeof(ui_vertex);
    cfg.vertex_alignment = NK_ALIGNOF(ui_vertex);
    cfg.circle_segment_count = 22;
    cfg.curve_segment_count = 22;
    cfg.arc_segment_count = 22;
    cfg.global_alpha = 1.0f;
    cfg.shape_AA = NK_ANTI_ALIASING_OFF;
    cfg.line_AA = NK_ANTI_ALIASING_OFF;
    nk_buffer_init_default(&ui.cmds);
    nk_buffer_init_fixed(&vbuf, ui.vertexbuffer, sizeof(ui_vertex)*BUFFER_LEN);
    nk_buffer_init_fixed(&ibuf, ui.indexbuffer, sizeof(unsigned short)*BUFFER_LEN);
    int returncode = nk_convert(&ui.context, &ui.cmds, &vbuf, &ibuf, &cfg);

    if(returncode == 0)
    {
        const nk_draw_command* cmd;
        size_t offset = 0;
        nk_draw_foreach(cmd, &ui.context, &ui.cmds)
        {
            if(cmd->elem_count == 0)
            {
                continue;
            }

            offset += cmd->elem_count;
            printf("offset:%zu\n", offset);
        }
    }

    nk_buffer_free(&ui.cmds);
    nk_clear(&ui.context);
}

void nk_ui_input()
{
    double x = 0;
    double y = 0;
    nk_input_begin(&ui.context);

    for(int i = 0; i < ui.text_length; i++)
    {
        nk_input_unicode(&ui.context, ui.text[i]);
    }

    ui.text_length = 0;
    nk_input_key(&ui.context, NK_KEY_DEL, glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_ENTER, glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_TAB, glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_BACKSPACE, glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_UP, glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_DOWN, glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_TEXT_START, glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_TEXT_END, glfwGetKey(window, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_SCROLL_START, glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_SCROLL_END, glfwGetKey(window, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_SCROLL_DOWN, glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_SCROLL_UP, glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS);
    nk_input_key(&ui.context, NK_KEY_SHIFT, glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                 glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
       glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
    {
        nk_input_key(&ui.context, NK_KEY_COPY, glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_PASTE, glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_CUT, glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_TEXT_UNDO, glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_TEXT_REDO, glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_TEXT_WORD_LEFT, glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_TEXT_WORD_RIGHT, glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_TEXT_LINE_START, glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_TEXT_LINE_END, glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
    }
    else
    {
        nk_input_key(&ui.context, NK_KEY_LEFT, glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_RIGHT, glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(&ui.context, NK_KEY_COPY, 0);
        nk_input_key(&ui.context, NK_KEY_PASTE, 0);
        nk_input_key(&ui.context, NK_KEY_CUT, 0);
        nk_input_key(&ui.context, NK_KEY_SHIFT, 0);
    }

    glfwGetCursorPos(window, &x, &y);
    nk_input_motion(&ui.context, (int)x, (int)y);
    nk_input_button(&ui.context, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    nk_input_button(&ui.context, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    nk_input_button(&ui.context, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    nk_input_button(&ui.context, NK_BUTTON_DOUBLE, (int)ui.mouse_double_x, (int)ui.mouse_double_y, ui.double_click_down);
    nk_input_scroll(&ui.context, ui.scroll);
    ui.scroll.x = 0;
    ui.scroll.y = 0;
    nk_input_end(&ui.context);
}

void nk_ui_update()
{
    nk_ui_input();
}
