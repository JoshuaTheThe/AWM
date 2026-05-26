
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
        if (Child)
        {
                Child->Parent = Parent;
        }

        if (Parent)
        {
                Child->Next   = Parent->Child;
                if (Parent->Child)
                        Parent->Child->Prev = Child;
                Parent->Child = Child;
        }
}

void AWM_RemoveWindow(AWM_Window *Window)
{
        if (Window->Next) Window->Next->Prev = Window->Prev;
        if (Window->Prev) Window->Prev->Next = Window->Next;
        if (Window->Parent && Window->Parent->Child == Window)
                Window->Parent->Child = Window->Next ? Window->Next : Window->Prev;
}

void AWM_RenderWindow(AWM_Window *Window)
{
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

                AWM_DrawRect(&Window->Surface, (AWM_Colour){.rgba_888_w=0x8025252f}, content_rect);
                AWM_DrawFilledCircleAA(&Window->Surface, (AWM_Colour){.rgba_888_w=0xfff53130}, 16, 15, 4);
                AWM_DrawFilledCircleAA(&Window->Surface, (AWM_Colour){.rgba_888_w=0xfff5f130}, 28, 15, 4);
                AWM_DrawFilledCircleAA(&Window->Surface, (AWM_Colour){.rgba_888_w=0xff31f530}, 40, 15, 4);
        }

        for (AWM_Window *Child = Window->Child; Child; Child = Child->Next)
        {
                AWM_RenderWindow(Child);
                AWM_BlitSurface(&Window->Surface, &Child->Surface);
        }
}
