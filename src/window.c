
#include <window.h>

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

