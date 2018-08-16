#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nuklearui.h"
#include "opengl.h"
#include "renderfunc.h"
#include "resources.h"
#include <unistd.h>
#include <stdalign.h>
#include "matrix.h"
#include "options.h"
#include "defines.h"

typedef struct nk_context nk_context;
typedef struct nk_font_atlas nk_font_atlas;
typedef struct nk_buffer nk_buffer;
typedef struct nk_draw_command nk_draw_command;

extern unsigned int currentprogram;

extern image* textures;
extern int numtextures;

#define MAX_TEXT 256

typedef struct ui_vertex
{
    float pos[3];
    float uv[2];
    unsigned char color[4];
} ui_vertex;

#define VERTEX_BUFFER_SIZE (65535 * sizeof(ui_vertex))
#define INDEX_BUFFER_SIZE (65535 * sizeof(unsigned short))

#define MAX_TEXTURES 16384

#define UI_TEXTURE_NONE 0
#define UI_TEXTURE_GL 1
#define UI_TEXTURE_IMAGE 2

typedef struct ui_texture
{
    char type;
    unsigned int id;
} ui_texture;

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
    ui_vertex* vertexbuffer;
    unsigned short* indexbuffer;
    int shader;
    unsigned int vertexarrayobject;
    unsigned int vertexbufferobject;
    unsigned int indexbufferobject;
    GLsync sync;
    ui_texture textures[MAX_TEXTURES];
    unsigned int numTextures;
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
    ui.textures[1].type = UI_TEXTURE_GL;
    glGenTextures(1, &ui.textures[1].id);
    glBindTexture(GL_TEXTURE_2D, ui.textures[1].id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    nk_font_atlas_init_default(&ui.font_atlas);
    int w = 0;
    int h = 0;
    const void* image = nk_font_atlas_bake(&ui.font_atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    nk_font_atlas_end(&ui.font_atlas, nk_handle_id(1), 0);
    nk_style_set_font(&ui.context, &ui.font_atlas.default_font->handle);
    ui.numTextures = 2;
}

void nk_ui_glinit()
{
    glCreateVertexArrays(1, &ui.vertexarrayobject);
    glCreateBuffers(1, &ui.vertexbufferobject);
    glCreateBuffers(1, &ui.indexbufferobject);
    GLbitfield flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    glNamedBufferStorage(ui.vertexbufferobject, VERTEX_BUFFER_SIZE, 0, flags);
    glNamedBufferStorage(ui.indexbufferobject, INDEX_BUFFER_SIZE, 0, flags);
    ui.indexbuffer = (unsigned short*)glMapNamedBufferRange(ui.indexbufferobject, 0, INDEX_BUFFER_SIZE, flags);
    ui.vertexbuffer = (ui_vertex*)glMapNamedBufferRange(ui.vertexarrayobject, 0, VERTEX_BUFFER_SIZE, flags);
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
    ui.shader = loadShader("shaders/ui/ui.vert", "shaders/ui/ui.frag", 0, 0, 0);
    nk_ui_glinit();

    if(ui.indexbuffer)
    {
        memset(ui.indexbuffer, 0, INDEX_BUFFER_SIZE);
    }

    if(ui.vertexbuffer)
    {
        memset(ui.vertexbuffer, 0, VERTEX_BUFFER_SIZE);
    }

    return &ui.context;
}

void nk_ui_destroy()
{
    nk_free(&ui.context);
}
void nk_ui_wait_for_buffer_unlock()
{
    if(!ui.sync)
    {
        return;
    }

    while(1)
    {
        GLenum wait = glClientWaitSync(ui.sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);

        if(wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED)
        {
            return;
        }
    }
}

void nk_ui_lock_buffer()
{
    if(ui.sync)
    {
        glDeleteSync(ui.sync);
    }

    ui.sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void nk_ui_render()
{
    nk_buffer vbuf = {0};
    nk_buffer ibuf = {0};
    struct nk_convert_config cfg = {0};
    static const struct nk_draw_vertex_layout_element vertex_layout[] =
    {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(ui_vertex, pos)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(ui_vertex, uv)},
        {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(ui_vertex, color)},
        {NK_VERTEX_LAYOUT_END}
    };
    cfg.vertex_layout = vertex_layout;
    cfg.vertex_size = sizeof(ui_vertex);
    cfg.vertex_alignment = alignof(ui_vertex);
    cfg.circle_segment_count = 22;
    cfg.curve_segment_count = 22;
    cfg.arc_segment_count = 22;
    cfg.global_alpha = 1.0f;
    cfg.shape_AA = NK_ANTI_ALIASING_ON;
    cfg.line_AA = NK_ANTI_ALIASING_OFF;
    nk_ui_wait_for_buffer_unlock();
    nk_buffer_init_default(&ui.cmds);
    nk_buffer_init_fixed(&vbuf, ui.vertexbuffer, VERTEX_BUFFER_SIZE);
    nk_buffer_init_fixed(&ibuf, ui.indexbuffer, INDEX_BUFFER_SIZE);
    int returncode = nk_convert(&ui.context, &ui.cmds, &vbuf, &ibuf, &cfg);

    if(returncode == 0)
    {
        glViewport(0, 0, options.width, options.height);
        const nk_draw_command* cmd;
        nk_draw_index* offset = 0;
        glBindVertexArray(ui.vertexarrayobject);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, ui.vertexbufferobject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui.indexbufferobject);
        bindShader(ui.shader);
        glBindAttribLocation(currentprogram, 0, "in_Position");
        glBindAttribLocation(currentprogram, 1, "in_Uvs");
        glBindAttribLocation(currentprogram, 2, "in_Color");
        mat4 ortho = {0};
        ortho.m[0] = 2.0f / (float)options.width;
        ortho.m[5] = -2.0f / (float)options.height;
        ortho.m[10] = -1.0f;
        ortho.m[12] = -1.0f;
        ortho.m[13] = 1.0f;
        ortho.m[15] = 1.0f;
        setUniformMat4("orthomat", &ortho);
        int texuniform = 0;
        glActiveTexture(GL_TEXTURE0);
        setUniformi("texture", &texuniform, 1);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);
        nk_draw_foreach(cmd, &ui.context, &ui.cmds)
        {
            if(cmd->elem_count == 0)
            {
                continue;
            }

            /*printf("texture:%i\n", cmd->texture.id);*/
            int hastex = 0;
            ui_texture* tex = &ui.textures[cmd->texture.id];

            switch(tex->type)
            {
                case UI_TEXTURE_NONE:
                {
                    glBindTexture(GL_TEXTURE_2D, 0);
                    break;
                }

                case UI_TEXTURE_GL:
                {
                    hastex = 1;
                    glBindTexture(GL_TEXTURE_2D, tex->id);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    break;
                }

                case UI_TEXTURE_IMAGE:
                {
                    hastex = 1;

                    if(tex->id < numtextures)
                    {
                        glBindTexture(GL_TEXTURE_2D, textures[tex->id].glImage);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }

                    break;
                }

                default:
                    break;
            }

            setUniformi("hastexture", &hastex, 1);
            glScissor(
                (GLint)(cmd->clip_rect.x),
                (GLint)((options.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h))),
                (GLint)(cmd->clip_rect.w),
                (GLint)(cmd->clip_rect.h));
            glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(ui_vertex), (void*)offsetof(ui_vertex, pos));
            glVertexAttribPointer(1, 2, GL_FLOAT, 0, sizeof(ui_vertex), (void*)offsetof(ui_vertex, uv));
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, 1, sizeof(ui_vertex), (void*)offsetof(ui_vertex, color));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        bindShader(-1);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
    }

    nk_buffer_free(&ui.cmds);
    nk_clear(&ui.context);
    nk_ui_lock_buffer();
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

struct nk_image nk_ui_image(char* filename)
{
    int image = loadImage(filename, IMG_TOP);
    ui.textures[ui.numTextures].type = UI_TEXTURE_IMAGE;
    ui.textures[ui.numTextures].id = image;
    struct nk_rect r = nk_rect(0, 0, textures[image].width, textures[image].height);
    struct nk_image out = nk_subimage_id(ui.numTextures,
                                         textures[image].width,
                                         textures[image].height,
                                         r);
    ui.numTextures++;
    return out;
}
