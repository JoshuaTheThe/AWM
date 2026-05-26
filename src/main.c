
#include<display.h>
#include<window.h>
#include<interact.h>
#include<cfg.h>
#include<font.h>

void AWM_ProcessMouse(AWM_Display *Display, AWM_Window *Window)
{
        if (Display->mouse_b & 0x01)
        {
                AWM_FindFocus(Display, Window, Display->mouse_x, Display->mouse_y);
                if (Display->Focus)
                {
                        AWM_RenderWindow(Display->Focus);
                        AWM_Present(&Display->Focus->Surface);
                }
        }
        if (Display->mouse_b & 0x02)
        {
                if (!Display->is_dragging && Display->Focus && Display->Focus != Window && Display->Focus->Type == AWM_WND_NORMAL)
                {
                        Display->drag_start_x = Display->mouse_x - Display->Focus->Surface.rect.x;
                        Display->drag_start_y = Display->mouse_y - Display->Focus->Surface.rect.y;
                        Display->is_dragging = 1;
                        Display->Cursor = AWM_CUR_MOVE;
                }
                else if (Display->is_dragging && Display->Focus && Display->Focus != Window && Display->Focus->Type == AWM_WND_NORMAL)
                {
                        int new_x = Display->mouse_x - Display->drag_start_x;
                        int new_y = Display->mouse_y - Display->drag_start_y;
                        new_x = MAX(0, MIN(new_x, (int)Display->Surface.rect.w - (int)Display->Focus->Surface.rect.w));
                        new_y = MAX(0, MIN(new_y, (int)Display->Surface.rect.h - (int)Display->Focus->Surface.rect.h));
                        if (new_x != Display->Focus->Surface.rect.x || new_y != Display->Focus->Surface.rect.y)
                        {
                                Display->Focus->Surface.rect.x = new_x;
                                Display->Focus->Surface.rect.y = new_y;
                                AWM_RenderWindow(Display->Focus);
                        }
                }
        }
        else
        {
                if (Display->is_dragging)
                {
                        Display->is_dragging = 0;
                        Display->Cursor = AWM_CUR_NORMAL;
                }
        }
        
        if (Display->mouse_b & 0x04)
        {
                if (!Display->is_resizing && Display->Focus && Display->Focus != Window && Display->Focus->Type == AWM_WND_NORMAL)
                {
                        Display->resize_start_x = Display->mouse_x;
                        Display->resize_start_y = Display->mouse_y;
                        Display->resize_start_w = Display->Focus->Surface.rect.w;
                        Display->resize_start_h = Display->Focus->Surface.rect.h;
                        Display->is_resizing = 1;
                        Display->Cursor = AWM_CUR_MOVE;
                }
                else if (Display->is_resizing && Display->Focus && Display->Focus != Window && Display->Focus->Type == AWM_WND_NORMAL)
                {
                        int delta_x = Display->mouse_x - Display->resize_start_x;
                        int delta_y = Display->mouse_y - Display->resize_start_y;
                        
                        int new_w = MAX(Display->resize_start_w + delta_x, 100);
                        int new_h = MAX(Display->resize_start_h + delta_y, 80);
                        
                        if (new_w != Display->Focus->Surface.rect.w || new_h != Display->Focus->Surface.rect.h)
                        {
                                int old_x = Display->Focus->Surface.rect.x;
                                int old_y = Display->Focus->Surface.rect.y;
                                size_t old_size = Display->Focus->Surface.rect.w * Display->Focus->Surface.rect.h * (Display->Focus->Surface.bpp >> 3);
                                size_t new_size = new_w * new_h * (Display->Focus->Surface.bpp >> 3);
                                void *new_back = AWM_New(new_size);
                                void *new_front = AWM_New(new_size);
                                
                                if (new_back && new_front)
                                {
                                        memset(new_back, 0, new_size);
                                        memset(new_front, 0, new_size);
                                        if (Display->Focus->Surface.back)
                                                AWM_Drop(Display->Focus->Surface.back, old_size);
                                        if (Display->Focus->Surface.front)
                                                AWM_Drop(Display->Focus->Surface.front, old_size);
                                        Display->Focus->Surface.back = new_back;
                                        Display->Focus->Surface.front = new_front;
                                        Display->Focus->Surface.rect.w = new_w;
                                        Display->Focus->Surface.rect.h = new_h;
                                        Display->Focus->Surface.rect.x = old_x;
                                        Display->Focus->Surface.rect.y = old_y;
                                        AWM_RenderWindow(Display->Focus);
                                        AWM_Present(&Display->Focus->Surface);
                                }
                        }
                }
        }
        else
        {
                if (Display->is_resizing)
                        {
                        Display->is_resizing = 0;
                        Display->Cursor = AWM_CUR_NORMAL;
                }
        }
}

int main(void)
{
        AWM_Display *Display   = AWM_OpenDisplay("/dev/fb0", "/dev/input/mouse0", "/dev/stdin", AWM_DEFAULT, AWM_DEFAULT, AWM_DEFAULT);
        AWM_Window  *Window    = AWM_NewWindow("ROOT", AWM_WND_ELEMENT, 0, 0, Display->Surface.rect.w, Display->Surface.rect.h, Display->vinfo.bits_per_pixel);
        AWM_Key     *Config    = AWM_ReadConfig("init.cfg");
        AWM_Surface  Backgr    = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/background-image")->Value);
        AWM_Surface  Cursor[4] = {
                [AWM_CUR_NORMAL] = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/normal")->Value),
                [AWM_CUR_AWAIT]  = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/await")->Value),
                [AWM_CUR_HELP]   = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/help")->Value),
                [AWM_CUR_MOVE]   = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/move")->Value),
        };

        Display->Font = AWM_LoadFont(AWM_ConfigFetch(Config, "AWM/system-font")->Value);
        Window->Font = Display->Font;

        AWM_Present(&Backgr);
        for (size_t i = 0; i < sizeof(Cursor) / sizeof(Cursor[0]); ++i)
                AWM_Present(&Cursor[i]);
        
        for (int i = 0; i < 4; ++i)
        {
                AWM_Window *w = AWM_NewWindow("Hello, World!", AWM_WND_NORMAL, i*128, 0, 128, 128, Display->vinfo.bits_per_pixel);
                w->Font = Display->Font;
                AWM_Window *t = AWM_NewWindow("This is Some Text", AWM_WND_TEXT, 16, AWM_TITLE_H, 140, 24, Display->vinfo.bits_per_pixel);
                t->Font = Display->Font;
                AWM_AttachChildWindow(Window, w);
                AWM_AttachChildWindow(w, t);
                AWM_RenderWindow(t);
                AWM_Present(&t->Surface);
                AWM_RenderWindow(w);
                AWM_Present(&w->Surface);
        }

        AWM_RenderWindow(Window);
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
                                case AWM_EV_KEY:
                                        if      (Event.as.Key == '+') (Display->Cursor = (Display->Cursor + 1) % 4);
                                        else if (Event.as.Key == '-') (Display->Cursor = (Display->Cursor + 3) % 4);
                                        break;
                                case AWM_EV_MOUSE_MOVE:
                                case AWM_EV_MOUSE_PRESS:
                                        break;
                                default:
                                        break;
                        }
                }

                AWM_ProcessMouse(Display, Window);
                AWM_Rect Rect = {.x=Display->mouse_x, .y=Display->mouse_y, 
                                 .w=Cursor[Display->Cursor].rect.w, .h=Cursor[Display->Cursor].rect.h};
                Cursor[Display->Cursor].rect = Rect;
                AWM_Clear(&Display->Surface);
                AWM_RenderWindow(Window);
                AWM_Present(&Window->Surface);
                AWM_BlitSurface(&Display->Surface, &Backgr);
                AWM_BlitSurface(&Display->Surface, &Window->Surface);
                AWM_BlitSurface(&Display->Surface, &Cursor[Display->Cursor]);
                AWM_Present(&Display->Surface);
        }

end:
        AWM_DestroyFont(Display->Font);
        AWM_CloseDisplay(Display);
        AWM_RemoveWindow(Window);
        AWM_DropWindow(Window);
        AWM_DropSurface(Backgr);
        for (size_t i = 0; i < sizeof(Cursor) / sizeof(Cursor[0]); ++i)
                AWM_DropSurface(Cursor[i]);
        AWM_FreeConfig(Config);
        return 0;
}
