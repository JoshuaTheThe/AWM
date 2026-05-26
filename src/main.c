
#include<display.h>
#include<window.h>
#include<interact.h>
#include<cfg.h>

int main(void)
{
        AWM_Display *Display   = AWM_OpenDisplay("/dev/fb0", "/dev/input/mouse0", "/dev/stdin", AWM_DEFAULT, AWM_DEFAULT, AWM_DEFAULT);
        AWM_Window  *Window    = AWM_NewWindow("ROOT", AWM_WND_ELEMENT, AWM_DEFAULT_XY, AWM_DEFAULT_XY, AWM_DEFAULT_WH, AWM_DEFAULT_WH, Display->vinfo.bits_per_pixel);
        AWM_Key     *Config    = AWM_ReadConfig("init.cfg");
        AWM_Surface  Backgr    = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/background-image")->Value);
        AWM_Surface  Cursor[4] = {
                [AWM_CUR_NORMAL] = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/normal")->Value),
                [AWM_CUR_AWAIT]  = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/await")->Value),
                [AWM_CUR_HELP]   = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/help")->Value),
                [AWM_CUR_MOVE]   = AWM_LoadImage(AWM_ConfigFetch(Config, "AWM/cursor/move")->Value),
        };

        int drag_start_x = 0,   drag_start_y = 0;
        int resize_start_x = 0, resize_start_y = 0;
        int resize_start_w = 0, resize_start_h = 0;
        int is_dragging = 0;
        int is_resizing = 0;
        
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
                                        if      (Event.as.Key == '+') (Display->Cursor = (Display->Cursor + 1) % 4);
                                        else if (Event.as.Key == '-') (Display->Cursor = (Display->Cursor + 3) % 4);
                                        break;
                                default:
                                        break;
                        }
                }

                if (Display->mouse_b & 0x01)
                {
                        AWM_FindFocus(Display, Window);
                        if (Display->Focus)
                        {
                                AWM_RenderWindow(Display->Focus);
                        }
                }

                if (Display->mouse_b & 0x02)
                {
                        if (!is_dragging && Display->Focus)
                        {
                                drag_start_x = Display->mouse_x - Display->Focus->Surface.rect.x;
                                drag_start_y = Display->mouse_y - Display->Focus->Surface.rect.y;
                                is_dragging = 1;
                                Display->Cursor = AWM_CUR_MOVE;
                        }
                        else if (is_dragging && Display->Focus)
                        {
                                int new_x = Display->mouse_x - drag_start_x;
                                int new_y = Display->mouse_y - drag_start_y;
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
                        if (is_dragging)
                        {
                                is_dragging = 0;
                                Display->Cursor = AWM_CUR_NORMAL;
                        }
                }
                
                if (Display->mouse_b & 0x04)
                {
                        if (!is_resizing && Display->Focus)
                        {
                                resize_start_x = Display->mouse_x;
                                resize_start_y = Display->mouse_y;
                                resize_start_w = Display->Focus->Surface.rect.w;
                                resize_start_h = Display->Focus->Surface.rect.h;
                                is_resizing = 1;
                                Display->Cursor = AWM_CUR_MOVE;
                        }
                        else if (is_resizing && Display->Focus)
                        {
                                int delta_x = Display->mouse_x - resize_start_x;
                                int delta_y = Display->mouse_y - resize_start_y;
                                
                                int new_w = MAX(resize_start_w + delta_x, 100);
                                int new_h = MAX(resize_start_h + delta_y, 80);
                                
                                if (new_w != Display->Focus->Surface.rect.w || new_h != Display->Focus->Surface.rect.h)
                                {
                                        // Save old position
                                        int old_x = Display->Focus->Surface.rect.x;
                                        int old_y = Display->Focus->Surface.rect.y;
                                        
                                        // Calculate new size
                                        size_t old_size = Display->Focus->Surface.rect.w * Display->Focus->Surface.rect.h * (Display->Focus->Surface.bpp >> 3);
                                        size_t new_size = new_w * new_h * (Display->Focus->Surface.bpp >> 3);
                                        
                                        // Allocate new surfaces
                                        void *new_back = AWM_New(new_size);
                                        void *new_front = AWM_New(new_size);
                                        
                                        if (new_back && new_front)
                                        {
                                                // Clear new surfaces to prevent garbage
                                                memset(new_back, 0, new_size);
                                                memset(new_front, 0, new_size);
                                                
                                                // Free old surfaces
                                                if (Display->Focus->Surface.back)
                                                        AWM_Drop(Display->Focus->Surface.back, old_size);
                                                if (Display->Focus->Surface.front)
                                                        AWM_Drop(Display->Focus->Surface.front, old_size);
                                                
                                                // Update window
                                                Display->Focus->Surface.back = new_back;
                                                Display->Focus->Surface.front = new_front;
                                                Display->Focus->Surface.rect.w = new_w;
                                                Display->Focus->Surface.rect.h = new_h;
                                                // Preserve position
                                                Display->Focus->Surface.rect.x = old_x;
                                                Display->Focus->Surface.rect.y = old_y;
                                                
                                                // Re-render the window with new size
                                                AWM_RenderWindow(Display->Focus);
                                                AWM_Present(&Display->Focus->Surface);
                                        }
                                }
                        }
                }
                else
                {
                        if (is_resizing)
                        {
                                is_resizing = 0;
                                Display->Cursor = AWM_CUR_NORMAL;
                        }
                }

                AWM_Rect Rect = {.x=Display->mouse_x, .y=Display->mouse_y, 
                                .w=Cursor[Display->Cursor].rect.w, .h=Cursor[Display->Cursor].rect.h};
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
