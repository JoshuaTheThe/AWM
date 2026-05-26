
#include <display.h>
#include <lib.h>

AWM_Display *AWM_OpenDisplay(const char *const Path, const char *const Mouse, long Width, long Height, long Bpp)
{
        AWM_Display *Display = new(AWM_Display);
        Display->framefp = AWM_Open(Path,  O_RDWR);
        Display->mousefp = AWM_Open(Mouse, O_RDONLY);

        // config
        ioctl(Display->framefp, FBIOGET_FSCREENINFO, &Display->finfo);
        ioctl(Display->framefp, FBIOGET_VSCREENINFO, &Display->vinfo);
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
        if (ioctl(Display->framefp, FBIOPUT_VSCREENINFO, &Display->vinfo) < 0)
                panic(PANIC_VINFO);
        ioctl(Display->framefp, FBIOGET_FSCREENINFO, &Display->finfo);
        ioctl(Display->framefp, FBIOGET_VSCREENINFO, &Display->vinfo);

        // map
        Display->fb = AWM_MMap(NULL, Display->finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, Display->framefp, 0);
        Display->back = AWM_New(Display->finfo.smem_len);

        // log
        fprintf(stderr, " [log] Created Display (%p)\n", (void *)Display);
        fprintf(stderr, " [log] Resolution: %dx%d\n", Display->vinfo.xres, Display->vinfo.yres);
        fprintf(stderr, " [log] Virtual resolution: %dx%d\n", Display->vinfo.xres_virtual, Display->vinfo.yres_virtual);
        fprintf(stderr, " [log] Bits per pixel: %d\n", Display->vinfo.bits_per_pixel);
        fprintf(stderr, " [log] Line length: %d bytes\n", Display->finfo.line_length);
        fprintf(stderr, " [log] Framebuffer size: %d bytes\n", Display->finfo.smem_len);

        return Display;
}

AWM_Event AWM_PollEvent(AWM_Display *Display)
{
        (void)Display;
        return (AWM_Event){0};
}

void AWM_CloseDisplay(AWM_Display *Display)
{
        if (!Display)
                panic(PANIC_NULL);
        AWM_Drop(Display->back, Display->finfo.smem_len);
        AWM_MUnMap(Display->fb, Display->finfo.smem_len);
        AWM_Close(Display->framefp);
        AWM_Close(Display->mousefp);
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

