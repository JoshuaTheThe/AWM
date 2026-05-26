
#include<interact.h>

void AWM_FindFocus(AWM_Display *Display, AWM_Window *RootWindow)
{
        if (!Display || !RootWindow)
                return;
        Display->Focus = RootWindow;
        for (AWM_Window *Window = RootWindow->Child; Window; Window = Window->Next)
        {
                if ((Display->mouse_x >= Window->Surface.rect.x) && 
                    (Display->mouse_y >= Window->Surface.rect.y) &&
                    (Display->mouse_x < Window->Surface.rect.x + Window->Surface.rect.w) && 
                    (Display->mouse_y < Window->Surface.rect.y + Window->Surface.rect.h))
                {
                        if (Window != RootWindow->Child)
                        {
                                AWM_RemoveWindow(Window);
                                AWM_AttachChildWindow(RootWindow, Window);
                        }
                        
                        Display->Focus = Window;
                        return;
                }
        }
}
