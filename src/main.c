
#include<display.h>
#include<window.h>
#include<cfg.h>

int main(void)
{
        AWM_Display *Display   = AWM_OpenDisplay("/dev/fb0", "/dev/input/mouse0", "/dev/stdin", AWM_DEFAULT, AWM_DEFAULT, AWM_DEFAULT);
        AWM_Window  *Window    = AWM_NewWindow("Hello, World!", AWM_WND_NORMAL, AWM_DEFAULT_XY, AWM_DEFAULT_XY, AWM_DEFAULT_WH, AWM_DEFAULT_WH, Display->vinfo.bits_per_pixel);
        AWM_Key     *Config    = AWM_ReadConfig("init.cfg");
        AWM_Surface  Backgr    = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/background-image")->Value);
        AWM_Surface  Cursor[4] = {
                [AWM_CUR_NORMAL] = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/normal")->Value),
                [AWM_CUR_AWAIT]  = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/await")->Value),
                [AWM_CUR_HELP]   = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/help")->Value),
                [AWM_CUR_MOVE]   = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/move")->Value),
        };

        AWM_RenderWindow(Window);
        AWM_Present(&Window->Surface);
        AWM_Present(&Backgr);
        for (size_t i = 0; i < sizeof(Cursor) / sizeof(Cursor[0]); ++i)
                AWM_Present(&Cursor[i]);

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
                                case AWM_EV_KEY:
                                        if      (Event.as.Key == '+') (Display->Cursor=Display->Cursor > 3 ? 0 : Display->Cursor+1);
                                        else if (Event.as.Key == '-') (Display->Cursor=Display->Cursor < 1 ? 3 : Display->Cursor-1);
                                        break;
                                default:
                                        break;
                        }
                }

                AWM_Rect Rect = {.x=Display->mouse_x, .y=Display->mouse_y, .w=Cursor[Display->Cursor].rect.w, .h=Cursor[Display->Cursor].rect.h};
                Cursor[Display->Cursor].rect = Rect;
                AWM_Clear(&Display->Surface);
                AWM_BlitSurface(&Display->Surface, &Backgr);
                AWM_BlitSurface(&Display->Surface, &Window->Surface);
                AWM_BlitSurface(&Display->Surface, &Cursor[Display->Cursor]);
                AWM_Present(&Display->Surface);
        }

end:
        AWM_CloseDisplay(Display);
        AWM_RemoveWindow(Window);
        AWM_DropWindow(Window);
        AWM_DropSurface(Backgr);
        for (size_t i = 0; i < sizeof(Cursor) / sizeof(Cursor[0]); ++i)
                AWM_DropSurface(Cursor[i]);
        AWM_FreeConfig(Config);
        return 0;
}

