
#ifndef AWM_DISPLAY_H
#define AWM_DISPLAY_H

#include<lib.h>
#include<linux/kd.h>
#include<linux/vt.h>
#include<termios.h>
#include<unistd.h>
#include<surface.h>

#define AWM_DEFAULT ((long)-1)
#define AWM_DEFAULT_VT (1)

typedef struct
{
        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;
        int            mouse_x;
        int            mouse_y;
        int            mouse_b;
        int            framefd;
        int            mousefd;
        int            ttyfd;
        int            stdin_flags;
        struct termios orig_termios;
        int            mouse_flags;
        struct termios orig_termios_mouse;
        AWM_Surface    Surface;
        bool           Running;
} AWM_Display;

typedef enum
{
        AWM_EV_QUIT,
        AWM_EV_MOUSE_MOVE,
        AWM_EV_MOUSE_PRESS,
        AWM_EV_KEY,
} AWM_EventKind;

typedef struct
{
        AWM_EventKind Kind;
        union {} as;
} AWM_Event;

AWM_Display *AWM_OpenDisplay(const char *const Path, const char *const Mouse, const char *const Keyboard, long Width, long Height, long Bpp);
bool AWM_PollEvent(AWM_Event *Event, AWM_Display *Display);
void AWM_CloseDisplay(AWM_Display *Display);

#endif

