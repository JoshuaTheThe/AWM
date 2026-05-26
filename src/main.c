
#include<display.h>
#include<window.h>
#include<cfg.h>

int main(void)
{
        AWM_Display *Display = AWM_OpenDisplay("/dev/fb0", "/dev/input/mouse0", "/dev/stdin", AWM_DEFAULT, AWM_DEFAULT, AWM_DEFAULT);
        AWM_Window  *Window  = AWM_NewWindow("Hello, World!", AWM_WND_NORMAL, AWM_DEFAULT_XY, AWM_DEFAULT_XY, AWM_DEFAULT_WH, AWM_DEFAULT_WH, Display->vinfo.bits_per_pixel);
        AWM_Key     *Config  = AWM_ReadConfig("init.cfg");
        AWM_Surface  Backgr  = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/background-image")->Value);
        AWM_Surface  Cursor  = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor-image")->Value);

        AWM_RenderWindow(Window);
        AWM_Present(&Window->Surface);
        AWM_Present(&Backgr);
        AWM_Present(&Cursor);

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

                AWM_Rect Rect = {.x=Display->mouse_x, .y=Display->mouse_y, .w=Cursor.rect.w, .h=Cursor.rect.h};
                Cursor.rect = Rect;
                AWM_Clear(&Display->Surface);
                AWM_BlitSurface(&Display->Surface, &Backgr);
                AWM_BlitSurface(&Display->Surface, &Window->Surface);
                AWM_BlitSurface(&Display->Surface, &Cursor);
                AWM_Present(&Display->Surface);
        }

end:
        AWM_CloseDisplay(Display);
        AWM_RemoveWindow(Window);
        AWM_DropWindow(Window);
        AWM_DropSurface(Backgr);
        AWM_DropSurface(Cursor);
        AWM_FreeConfig(Config);
        return 0;
}

