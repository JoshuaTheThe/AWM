
#ifndef WINDOW_H
#define WINDOW_H

#include <lib.h>
#include <surface.h>
#include <display.h>

#define AWM_DEFAULT_XY (128)
#define AWM_DEFAULT_WH (256)
#define AWM_TITLE_LEN  (256)
#define AWM_TITLE_H    (16)

typedef enum
{
        AWM_WND_NORMAL,
        AWM_WND_ELEMENT,
} AWM_WindowType;

typedef struct AWM_Window
{
        char               Title[AWM_TITLE_LEN];
        struct AWM_Window *Parent,*Child,
                          *Next,*Prev;
        AWM_Surface        Surface;
        AWM_WindowType     Type;
} AWM_Window;

AWM_Window *AWM_NewWindow(const char *const Title, AWM_WindowType Type,
                          size_t X,size_t Y,
                          size_t W,size_t H,
                          size_t Bpp);
void AWM_DropWindow(AWM_Window *Window);
void AWM_AttachChildWindow(AWM_Window *Parent, AWM_Window *Child);
void AWM_RenderWindow(AWM_Window *Window);
void AWM_RemoveWindow(AWM_Window *Window);

#endif

