
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
        Display->Surface       = AWM_NewSurface(Display->vinfo.xres, Display->vinfo.yres, Display->vinfo.bits_per_pixel);
        Display->Surface.front = AWM_MMap(NULL, Display->finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, Display->framefd, 0);

        // tty
        int flags = fcntl(Display->ttyfd, F_GETFL, 0);
        Display->stdin_flags = flags;
        fcntl(Display->ttyfd, F_SETFL, flags | O_NONBLOCK);
        struct termios raw;
        tcgetattr(Display->ttyfd, &Display->orig_termios);
        raw = Display->orig_termios;
        raw.c_lflag &= ~(ICANON | ECHO);
        //raw.c_iflag &= ~(IXON | INPCK | ISTRIP);
        //raw.c_oflag &= ~(OPOST);
        //raw.c_cflag |= (CS8);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(Display->ttyfd, TCSANOW, &raw);

        // mouse
        flags = fcntl(Display->mousefd, F_GETFL, 0);
        Display->mouse_flags = flags;
        fcntl(Display->mousefd, F_SETFL, flags | O_NONBLOCK);
        tcgetattr(Display->mousefd, &Display->orig_termios_mouse);
        raw = Display->orig_termios_mouse;
        raw.c_lflag &= ~(ICANON | ECHO);
        //raw.c_iflag &= ~(IXON | INPCK | ISTRIP);
        //raw.c_oflag &= ~(OPOST);
        //raw.c_cflag |= (CS8);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(Display->mousefd, TCSANOW, &raw);

        // so kernel doesn't draw over us
        ioctl(Display->ttyfd, KDSETMODE, KD_GRAPHICS);
        
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
        char buf[1024] = {0};
        bytes = read(Display->mousefd, buf, 3);
        if (bytes >= 3)
        {
                Display->mouse_b  = buf[0];
                Display->mouse_x += buf[1];
                Display->mouse_y -= buf[2];
                if (Display->mouse_x < 0)
                        Display->mouse_x = 0;
                if (Display->mouse_y < 0)
                        Display->mouse_y = 0;
                if (Display->mouse_x >= Display->vinfo.xres)
                        Display->mouse_x = Display->vinfo.xres - 1;
                if (Display->mouse_y >= Display->vinfo.yres)
                        Display->mouse_y = Display->vinfo.yres - 1;
                Event->Kind = AWM_EV_MOUSE_MOVE;
                return true;
        }

        memset(buf, 0, sizeof(buf));
        bytes = read(Display->ttyfd, &buf, 1023);
        if (bytes >= 0)
        {
                for (size_t i = 0; i < sizeof(buf) && buf[i]; ++i)
                {
                        if (buf[i] == 'q')
                        {
                                Event->Kind = AWM_EV_QUIT;
                                return true;
                        }
                        
                        Event->Kind   = AWM_EV_KEY;
                        Event->as.Key = buf[i];
                        return true;
                }
        }

        return false;
}

void AWM_CloseDisplay(AWM_Display *Display)
{
        if (!Display)
                panic(PANIC_NULL);
        ioctl(Display->ttyfd, KDSETMODE, KD_TEXT);
        fcntl(Display->ttyfd, F_SETFL, Display->stdin_flags);
        tcsetattr(Display->mousefd, TCSANOW, &Display->orig_termios_mouse);
        tcsetattr(Display->ttyfd, TCSANOW, &Display->orig_termios);
        AWM_DropSurface(Display->Surface);
        AWM_Close(Display->framefd);
        AWM_Close(Display->mousefd);
        AWM_Close(Display->ttyfd);
        drop(Display);
}
