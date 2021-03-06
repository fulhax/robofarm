#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#define CLEAN_USED    0
#define CLEAN_DELETED 1
#define CLEAN_LATER   2

typedef struct image
{
    char* name;
    unsigned int glImage;
    int width;
    int height;
    int channels; /*1-4*/
    char origin;
} image;

typedef struct shader
{
    char* controlname;
    char* evalname;
    char* fragname;
    char* geomname;
    char* vertname;
    unsigned int control;
    unsigned int eval;
    unsigned int frag;
    unsigned int geom;
    unsigned int vert;
    unsigned int program;
    char working;
} shader;

int loadImage(const char* filename, char origin);
int loadShader(const char* vertex, const char* pixel, const char* geometry, const char* tesscontrol, const char* tesseval);
void reloadImage(const char* filename);
void reloadShader(const char* filename);
void cleanupImages();
void cleanupShaders();
void initImages();

void initResourceCleanup();
void endResourceCleanup();

#endif //__RESOURCES_H__
