
#include <window.h>
#include <display.h>

AWM_Window *AWM_NewWindow(const char *const Title, AWM_WindowType Type,
                          size_t X,size_t Y,
                          size_t W,size_t H,
                          size_t Bpp)
{
        AWM_Window *Window     = new(AWM_Window);
        Window->Surface        = AWM_NewSurface(W,H,Bpp);
        Window->Surface.front  = AWM_New(W*H*(Bpp >> 3));
        Window->Surface.rect.x = X;
        Window->Surface.rect.y = Y;
        Window->Type           = Type;
        strncpy(Window->Title, Title, AWM_TITLE_LEN - 1);
        return Window;
}

void AWM_DropWindow(AWM_Window *Window)
{
        if (!Window)
                return;
        AWM_DropSurface(Window->Surface);
        AWM_DropWindow(Window->Next);
        AWM_DropWindow(Window->Child);
        drop(Window);
}

void AWM_AttachChildWindow(AWM_Window *Parent, AWM_Window *Child)
{
        if (!Child)
                return;
        if (Child->Parent)
                AWM_RemoveWindow(Child);
        Child->Parent = Parent;
        if (Parent)
        {
                Child->Next = Parent->Child;
                if (Parent->Child)
                        Parent->Child->Prev = Child;
                Parent->Child = Child;
                Child->Prev = NULL;
        }
}

void AWM_RemoveWindow(AWM_Window *Window)
{
        if (!Window)
                return;
        if (Window->Next)
                Window->Next->Prev = Window->Prev;
        if (Window->Prev)
                Window->Prev->Next = Window->Next;
        if (Window->Parent && Window->Parent->Child == Window)
                Window->Parent->Child = Window->Next;
        Window->Prev = NULL;
        Window->Next = NULL;
}

void AWM_RenderWindow(AWM_Window *Window)
{
        if (!Window)
                return;
                
        AWM_Clear(&Window->Surface);
        
        if (Window->Type == AWM_WND_NORMAL)
        {
                float corner_radius = 25.0f;
                float squircle_exp = 4.0f;
                AWM_Rect shadow_rect =
                {
                        .x = 2, .y = 2,
                        .w = Window->Surface.rect.w - 2,
                        .h = Window->Surface.rect.h - 2
                };

                AWM_DrawFilledSquircle(&Window->Surface, 
                                       (AWM_Colour){.rgba_888_w=0x7015151f}, 
                                       shadow_rect, corner_radius, squircle_exp);
                AWM_Rect content_rect =
                {
                        .x = 8, 
                        .y = AWM_TITLE_H,
                        .w = Window->Surface.rect.w - 16,
                        .h = Window->Surface.rect.h - AWM_TITLE_H - 8
                };

                AWM_Colour fg[4] = {{.rgba_888_w=0x90a0a0a0},{.rgba_888_w=0xffa0a0a0},{.rgba_888_w=0xffe0e0e0},{.rgba_888_w=0xffffffff}};
                for (size_t i = 0; Window->Title[i] && i < AWM_TITLE_LEN; ++i)
                        AWM_DrawCharacter(&Window->Surface, (Window->Font->char_width * i) + 64, 8, Window->Title[i], fg, Window->Font);
                AWM_DrawRect(&Window->Surface, (AWM_Colour){.rgba_888_w=0x8025252f}, content_rect);
                AWM_DrawFilledCircleAA(&Window->Surface, (AWM_Colour){.rgba_888_w=0xfff53130}, 16, 15, 4);
                AWM_DrawFilledCircleAA(&Window->Surface, (AWM_Colour){.rgba_888_w=0xfff5f130}, 28, 15, 4);
                AWM_DrawFilledCircleAA(&Window->Surface, (AWM_Colour){.rgba_888_w=0xff31f530}, 40, 15, 4);
        }
        else if (Window->Type == AWM_WND_TEXT)
        {
                AWM_Colour fg[4] = {{.rgba_888_w=0x90a0a0a0},{.rgba_888_w=0xffa0a0a0},{.rgba_888_w=0xffe0e0e0},{.rgba_888_w=0xffffffff}};
                for (size_t i = 0; Window->Title[i] && i < AWM_TITLE_LEN; ++i)
                        AWM_DrawCharacter(&Window->Surface, Window->Font->char_width * i, 8, Window->Title[i], fg, Window->Font);
        }
        
        AWM_Window *last = Window->Child;
        if (last)
        {
                while (last->Next)
                        last = last->Next;
                for (AWM_Window *Child = last; Child; Child = Child->Prev)
                {
                        AWM_RenderWindow(Child);
                        AWM_BlitSurface(&Window->Surface, &Child->Surface);
                }
        }
}
