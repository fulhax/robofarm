#ifndef __OPTIONS_H__
#define __OPTIONS_H__

typedef struct robofarm_options
{
    int x, y, width, height;
} robofarm_options;

int handle_options(int argc, char* argv[]);

extern robofarm_options options;

#endif //__OPTIONS_H__
