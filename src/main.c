
#include<display.h>

void pixel(void *buf, size_t bpp,
           size_t w,  size_t h,
           size_t x,  size_t y,
           uint32_t col)
{
        for (size_t i = 0; i < bpp >> 3; ++i)
        {
                ((unsigned char*)buf)[x + y * w] = col & 255;
                col >>= 8;
        }
}

int main(void)
{
        AWM_Display *Display = AWM_OpenDisplay("/dev/fb0", "/dev/input/mouse0", AWM_DEFAULT, AWM_DEFAULT, AWM_DEFAULT);
        AWM_Rect     Rect = {.x=8,.y=8,.w=128,.h=128};
        AWM_Colour   Colour = {.rgba_888_w = 0xffffffff};
        AWM_DrawRect(Display, Colour, Rect);
        AWM_Present(Display);
        fgetc(stdin);
        AWM_CloseDisplay(Display);
        return 0;
}

