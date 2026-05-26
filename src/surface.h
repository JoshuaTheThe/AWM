
#ifndef SURFACE_H
#define SURFACE_H

#include <lib.h>

#define MIN(a,b) (a > b ? b : a)
#define MAX(a,b) (a > b ? a : b)
#define AWM_COPY_DIM(R) ((AWM_Rect){.x=0,.y=0,.w=R.w,.h=R.h})
#define AWM_RECT(W,H)   ((AWM_Rect){.x=0,.y=0,.w=W,.h=H})
#define AWM_RECTXY(X,Y,W,H)   ((AWM_Rect){.x=X,.y=Y,.w=W,.h=H})

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
        size_t x,y,w,h;
} AWM_Rect;

typedef struct
{
        void    *front,*back;
        AWM_Rect rect;
        size_t   bpp;
} AWM_Surface;

AWM_Surface AWM_LoadImage(const char *filename);
AWM_Surface AWM_NewSurface(size_t W,size_t H, size_t Bpp);
void        AWM_DropSurface(AWM_Surface Surface);
void        AWM_Present(AWM_Surface *Surface);
void        AWM_Clear(AWM_Surface *Surface);
void        AWM_DrawRect(AWM_Surface *Dest, AWM_Colour Colour, AWM_Rect Rect);
void        AWM_DrawPoint(AWM_Surface *Dest, AWM_Colour Colour, size_t X, size_t Y);
void        AWM_DrawLine(AWM_Surface *Dest, AWM_Colour Colour, size_t X1, size_t Y1, size_t X2, size_t Y2);
void        AWM_BlitSurface(AWM_Surface *Dest, AWM_Surface *Source);
void        AWM_DrawFilledSquircle(AWM_Surface *Surface, AWM_Colour Colour, 
                                   AWM_Rect Rect, float Radius, float Exponent);
void        AWM_DrawCircleAA(AWM_Surface *Dest, AWM_Colour Colour,
                             size_t CenterX, size_t CenterY, size_t Radius, int Thickness);
void        AWM_DrawCircle(AWM_Surface *Dest, AWM_Colour Colour,
                           size_t CenterX, size_t CenterY, size_t Radius, int Thickness);
void        AWM_DrawFilledCircleAA(AWM_Surface *Dest, AWM_Colour Colour,
                                   size_t CenterX, size_t CenterY, size_t Radius);
void        AWM_DrawFilledCircle(AWM_Surface *Dest, AWM_Colour Colour, 
                                 size_t CenterX, size_t CenterY, size_t Radius);

#endif
