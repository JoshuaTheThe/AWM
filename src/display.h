
#ifndef AWM_DISPLAY_H
#define AWM_DISPLAY_H

#include<lib.h>

#define AWM_DEFAULT ((long)-1)

typedef union __attribute__((__packed__))
{
        struct
        {
                char r;
                char g;
                char b;
                char a;
        } rgba_888;

        struct
        {
                char r;
                char g;
                char b;
        } rgba_888_24;

        struct
        {
                char r : 5;
                char g : 5;
                char b : 5;
                char a : 1;
        } rgba_555;

        struct
        {
                char entry;
        } palette_256;

        int   rgba_888_w;
        short rgba_555_w;
        char  palette_256_w;
} AWM_Colour;

typedef struct
{
        int x,y,w,h;
} AWM_Rect;

typedef struct
{
        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;
        int         mouse_x;
        int         mouse_y;
        int         mouse_b;
        int         framefp;
        int         mousefp;
        void       *fb;
        void       *back;
} AWM_Display;


typedef struct
{
        void *placeholder;
} AWM_Event;

AWM_Display *AWM_OpenDisplay(const char *const Path, const char *const Mouse, long Width, long Height, long Bpp);
AWM_Event AWM_PollEvent(AWM_Display *Display);
void AWM_CloseDisplay(AWM_Display *Display);
void AWM_DrawRect(AWM_Display *Display, AWM_Colour Colour, AWM_Rect Rect);
void AWM_Present(AWM_Display *Display);

#endif

