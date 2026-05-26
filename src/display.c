
#include <display.h>
#include <lib.h>

AWM_Display *AWM_OpenDisplay(const char *const Path, const char *const Mouse, const char *const TTY, long Width, long Height, long Bpp)
{
        AWM_Display *Display = new(AWM_Display);
        Display->framefd     = AWM_Open(Path,  O_RDWR);
        Display->mousefd     = AWM_Open(Mouse, O_RDONLY);
        Display->ttyfd       = AWM_Open(TTY, O_RDWR);
        Display->Running     = true;

        // config
        ioctl(Display->framefd, FBIOGET_FSCREENINFO, &Display->finfo);
        ioctl(Display->framefd, FBIOGET_VSCREENINFO, &Display->vinfo);
        if (Width != AWM_DEFAULT)
        {
                Display->vinfo.xres = Width;
                Display->vinfo.xres_virtual = Width;
        }
        if (Height != AWM_DEFAULT)
        {
                Display->vinfo.yres = Height;
                Display->vinfo.yres_virtual = Height;
        }
        if (Bpp != AWM_DEFAULT)
                Display->vinfo.bits_per_pixel = Bpp;
        if (ioctl(Display->framefd, FBIOPUT_VSCREENINFO, &Display->vinfo) < 0)
                panic(PANIC_VINFO);
        ioctl(Display->framefd, FBIOGET_FSCREENINFO, &Display->finfo);
        ioctl(Display->framefd, FBIOGET_VSCREENINFO, &Display->vinfo);

        // map
        Display->fb = AWM_MMap(NULL, Display->finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, Display->framefd, 0);
        Display->back = AWM_New(Display->finfo.smem_len);

        int flags = fcntl(Display->ttyfd, F_GETFL, 0);
        Display->stdin_flags = flags;
        fcntl(Display->ttyfd, F_SETFL, flags | O_NONBLOCK);

        // so kernel doesn't draw over us
        int vt;
        ioctl(Display->ttyfd, VT_GETACTIVE, &vt);
        ioctl(Display->ttyfd, VT_ACTIVATE, vt);
        ioctl(Display->ttyfd, VT_WAITACTIVE, vt);
        ioctl(Display->ttyfd, KDSETMODE, KD_GRAPHICS);
        Display->old_vt = vt;
        
        // log
        fprintf(stderr, " [log] Created Display (%p)\n", (void *)Display);
        fprintf(stderr, " [log] Resolution: %dx%d\n", Display->vinfo.xres, Display->vinfo.yres);
        fprintf(stderr, " [log] Virtual resolution: %dx%d\n", Display->vinfo.xres_virtual, Display->vinfo.yres_virtual);
        fprintf(stderr, " [log] Bits per pixel: %d\n", Display->vinfo.bits_per_pixel);
        fprintf(stderr, " [log] Line length: %d bytes\n", Display->finfo.line_length);
        fprintf(stderr, " [log] Framebuffer size: %d bytes\n", Display->finfo.smem_len);

        return Display;
}

bool AWM_PollEvent(AWM_Event *Event, AWM_Display *Display)
{
        int bytes;
        ioctl(Display->mousefd, FIONREAD, &bytes);
        if (bytes >= 3)
        {
                char buf[3];
                read(Display->mousefd, buf, 3);
                if (buf[1] != 0 && buf[2] != 0)
                {
                        Display->mouse_b  = buf[0];
                        Display->mouse_x += buf[1];
                        Display->mouse_y += buf[2];
                        Event->Kind = AWM_EV_MOUSE_MOVE;
                        return true;
                }
                else if (buf[0] != Display->mouse_b)
                {
                        Display->mouse_b = buf[0];
                        Event->Kind      = AWM_EV_MOUSE_PRESS;
                        return true;
                }
        }

        ioctl(Display->ttyfd, FIONREAD, &bytes);
        if (bytes == 1)
        {
                char buf;
                read(Display->ttyfd, &buf, 1);
                if (buf == 'q')
                {
                        Event->Kind = AWM_EV_QUIT;
                }

                return true;
        }

        return false;
}

void AWM_CloseDisplay(AWM_Display *Display)
{
        if (!Display)
                panic(PANIC_NULL);
        ioctl(Display->ttyfd, KDSETMODE, KD_TEXT);
        ioctl(Display->ttyfd, VT_ACTIVATE, Display->old_vt);
        ioctl(Display->ttyfd, VT_WAITACTIVE, Display->old_vt);
        AWM_Drop(Display->back, Display->finfo.smem_len);
        AWM_MUnMap(Display->fb, Display->finfo.smem_len);
        AWM_Close(Display->framefd);
        AWM_Close(Display->mousefd);
        fcntl(Display->ttyfd, F_SETFL, Display->stdin_flags);
        AWM_Close(Display->ttyfd);
        AWM_Drop(Display);
}

void AWM_DrawRect(AWM_Display *Display, AWM_Colour Colour, AWM_Rect Rect)
{
        const size_t _my = Rect.y + Rect.h;
        const size_t _mx = Rect.x + Rect.w;
        const size_t my  = _my >= Display->vinfo.yres ? Display->vinfo.yres - 1 : _my;
        const size_t mx  = _mx >= Display->vinfo.xres ? Display->vinfo.xres - 1 : _mx;
        for (size_t y = Rect.y; y < my; ++y)
        {
                const size_t rowOff = y * Display->vinfo.xres;
                for (size_t x = Rect.x; x <  mx; ++x)
                {
                        switch (Display->vinfo.bits_per_pixel)
                        {
                                case 32:
                                        ((int *)Display->back)[x+rowOff] = Colour.rgba_888_w;
                                        break;
                                case 24:
                                        ((char *)Display->back)[x+rowOff+0] = Colour.rgba_888_24.r;
                                        ((char *)Display->back)[x+rowOff+1] = Colour.rgba_888_24.g;
                                        ((char *)Display->back)[x+rowOff+2] = Colour.rgba_888_24.b;
                                        break;
                                case 16:
                                        ((short *)Display->back)[x+rowOff] = Colour.rgba_555_w;
                                        break;
                                case 8:
                                        ((char *)Display->back)[x+rowOff] = Colour.palette_256_w;
                                        break;
                        }
                }
        }
}

void AWM_Present(AWM_Display *Display)
{
        memcpy(Display->fb, Display->back, Display->finfo.smem_len);
}

