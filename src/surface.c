
#include <surface.h>

AWM_Surface AWM_NewSurface(size_t W,size_t H, size_t Bpp)
{
        void *p = AWM_New(W*H*(Bpp >> 3));
        return (AWM_Surface){.bpp = Bpp, .rect={.x=0,.y=0,.w=W,.h=H}, .back=p, .front=NULL};
}

void AWM_DropSurface(AWM_Surface Surface)
{
        AWM_Drop(Surface.front, Surface.rect.w * Surface.rect.h * (Surface.bpp >> 3));
        AWM_Drop(Surface.back,  Surface.rect.w * Surface.rect.h * (Surface.bpp >> 3));
}

void AWM_Present(AWM_Surface *Surface)
{
        memcpy(Surface->front, Surface->back, Surface->rect.w * Surface->rect.h * (Surface->bpp >> 3));
}

void AWM_Clear(AWM_Surface *Surface)
{
        memset(Surface->back, 0, Surface->rect.w * Surface->rect.h * (Surface->bpp >> 3));
}

void AWM_DrawRect(AWM_Surface *Dest, AWM_Colour Colour, AWM_Rect Rect)
{
        const size_t _my = Rect.y + Rect.h;
        const size_t _mx = Rect.x + Rect.w;
        const size_t my  = _my >= Dest->rect.h ? Dest->rect.h - 1 : _my;
        const size_t mx  = _mx >= Dest->rect.w ? Dest->rect.w - 1 : _mx;
        for (size_t y = Rect.y; y < my; ++y)
        {
                for (size_t x = Rect.x; x <  mx; ++x)
                {
                        AWM_DrawPoint(Dest, Colour, x, y);
                }
        }
}

void AWM_DrawPoint(AWM_Surface *Dest, AWM_Colour Colour, size_t x, size_t y)
{
        const size_t rowOff = y * Dest->rect.w;
        switch (Dest->bpp)
        {
        case 32:
                ((int *)Dest->back)[x+rowOff] = Colour.rgba_888_w;
                break;
        case 24:
                ((char *)Dest->back)[x+(rowOff*3)+0] = Colour.rgba_888_24.r;
                ((char *)Dest->back)[x+(rowOff*3)+1] = Colour.rgba_888_24.g;
                ((char *)Dest->back)[x+(rowOff*3)+2] = Colour.rgba_888_24.b;
                break;
        case 16:
                ((short *)Dest->back)[x+rowOff] = Colour.rgba_555_w;
                break;
        case 8:
                ((char *)Dest->back)[x+rowOff] = Colour.palette_256_w;
                break;
        }
}

void AWM_DrawLine(AWM_Surface *Dest, AWM_Colour Colour, size_t X1, size_t Y1, size_t X2, size_t Y2);

void AWM_BlitSurface(AWM_Surface *Dest, AWM_Surface *Surface)
{
        if (!Dest || !Surface)
                return;
        if (Dest->bpp != Surface->bpp)
                return;
        if (!Dest->back || !Surface->front)
                return;
        int64_t dst_x = Surface->rect.x;
        int64_t dst_y = Surface->rect.y;
        if (dst_x >= (int64_t)Dest->rect.w || dst_y >= (int64_t)Dest->rect.h)
                return;
        if (dst_x + (int64_t)Surface->rect.w <= 0 || dst_y + (int64_t)Surface->rect.h <= 0)
                return;
        int64_t start_row = 0;
        int64_t end_row = Surface->rect.h;
        int64_t dest_start_y = dst_y;
        
        if (dst_y < 0)
        {
                start_row = -dst_y;
                dest_start_y = 0;
        }
        
        if (dest_start_y + (end_row - start_row) > (int64_t)Dest->rect.h)
        {
                end_row = start_row + ((int64_t)Dest->rect.h - dest_start_y);
        }
        
        size_t bytes_per_pixel = Surface->bpp / 8;
        size_t src_stride = Surface->rect.w * bytes_per_pixel;
        size_t dst_stride = Dest->rect.w * bytes_per_pixel;
        for (int64_t row = start_row; row < end_row; ++row)
        {
                int64_t src_x = 0;
                int64_t dest_start_x = dst_x;
                size_t copy_width = Surface->rect.w;
                
                if (dst_x < 0)
                {
                        src_x = -dst_x;
                        dest_start_x = 0;
                        copy_width = Surface->rect.w - src_x;
                }
                
                if (dest_start_x + (int64_t)copy_width > (int64_t)Dest->rect.w)
                {
                        copy_width = Dest->rect.w - dest_start_x;
                }
                
                if (copy_width == 0)
                        continue;
                uint8_t *src_ptr = (uint8_t*)Surface->front;
                src_ptr += (row * src_stride) + (src_x * bytes_per_pixel);
                uint8_t *dst_ptr = (uint8_t*)Dest->back;
                dst_ptr += (dest_start_y + (row - start_row)) * dst_stride + (dest_start_x * bytes_per_pixel);
                memcpy(dst_ptr, src_ptr, copy_width * bytes_per_pixel);
        }
}
