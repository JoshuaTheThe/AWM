
#include<interact.h>

AWM_Window* AWM_FindFocusRecursive(AWM_Window *Window, size_t global_x, size_t global_y)
{
        if (!Window)
                return NULL;
        int inside = (global_x >= Window->Surface.rect.x) && 
                     (global_y >= Window->Surface.rect.y) &&
                     (global_x < Window->Surface.rect.x + (int)Window->Surface.rect.w) && 
                     (global_y < Window->Surface.rect.y + (int)Window->Surface.rect.h);
        if (!inside)
                return NULL;
        size_t local_x = global_x - Window->Surface.rect.x;
        size_t local_y = global_y - Window->Surface.rect.y;
        for (AWM_Window *Child = Window->Child; Child; Child = Child->Next)
        {
                int child_inside = (local_x >= Child->Surface.rect.x) && 
                                   (local_y >= Child->Surface.rect.y) &&
                                   (local_x < Child->Surface.rect.x + (int)Child->Surface.rect.w) && 
                                   (local_y < Child->Surface.rect.y + (int)Child->Surface.rect.h);
                
                if (child_inside)
                {
                        AWM_Window *focused = AWM_FindFocusRecursive(Child, local_x, local_y);
                        if (focused)
                                return focused;
                        return Child;
                }
        }
        
        return Window;
}

void AWM_FindFocus(AWM_Display *Display, AWM_Window *RootWindow, size_t x, size_t y)
{
        if (!Display || !RootWindow)
                return;
        
        AWM_Window *focused = AWM_FindFocusRecursive(RootWindow, x, y);
        
        if (focused && focused != RootWindow)
        {
                if (focused != RootWindow->Child && focused->Type == AWM_WND_NORMAL)
                {
                        AWM_RemoveWindow(focused);
                        AWM_AttachChildWindow(RootWindow, focused);
                }
                Display->Focus = focused;
        }
        else
        {
                Display->Focus = RootWindow;
        }
}
