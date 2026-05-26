
#include<display.h>
#include<window.h>

int main(void)
{
        AWM_Display *Display = AWM_OpenDisplay("/dev/fb0", "/dev/input/mouse0", "/dev/stdin", AWM_DEFAULT, AWM_DEFAULT, AWM_DEFAULT);
        AWM_Window  *Window  = AWM_NewWindow("Hello, World!", AWM_WND_NORMAL, AWM_DEFAULT_XY, AWM_DEFAULT_XY, AWM_DEFAULT_WH, AWM_DEFAULT_WH, Display->vinfo.bits_per_pixel);
        AWM_Rect Rect = {.x=0, .y=0, .w=Window->Surface.rect.w, .h=Window->Surface.rect.h};
        AWM_Clear(&Window->Surface);
        AWM_DrawRect(&Window->Surface, (AWM_Colour){.rgba_888_w=0xff008080}, Rect);
        AWM_Present(&Window->Surface);
        while (Display->Running)
        {
                AWM_Event Event = {0};
                while (AWM_PollEvent(&Event, Display))
                {
                        switch (Event.Kind)
                        {
                                case AWM_EV_QUIT:
                                        Display->Running = false;
                                        goto end;
                                default:
                                        break;
                        }
                }

                AWM_Rect Rect = {.x=Display->mouse_x, .y=Display->mouse_y, .w=8, .h=8};
                AWM_Clear(&Display->Surface);
                AWM_BlitSurface(&Display->Surface, &Window->Surface);
                AWM_DrawRect(&Display->Surface, (AWM_Colour){.rgba_888_w=0xff808080}, Rect);
                AWM_Present(&Display->Surface);
        }

end:
        AWM_CloseDisplay(Display);
        AWM_RemoveWindow(Window);
        AWM_DropWindow(Window);
        return 0;
}

