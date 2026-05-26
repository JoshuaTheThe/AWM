
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <surface.h>

static uint32_t blend_pixel(uint32_t dst, uint32_t src)
{
        uint8_t src_a = (src >> 24) & 0xFF;
        if (src_a == 0) return dst;
        if (src_a == 255) return src;
        uint8_t dst_a = (dst >> 24) & 0xFF;
        uint8_t out_a = src_a + (dst_a * (255 - src_a) / 255);
        uint8_t src_r = (src >> 16) & 0xFF;
        uint8_t src_g = (src >> 8) & 0xFF;
        uint8_t src_b = src & 0xFF;
        uint8_t dst_r = (dst >> 16) & 0xFF;
        uint8_t dst_g = (dst >> 8) & 0xFF;
        uint8_t dst_b = dst & 0xFF;
        uint8_t out_r = (src_r * src_a + dst_r * (255 - src_a)) / 255;
        uint8_t out_g = (src_g * src_a + dst_g * (255 - src_a)) / 255;
        uint8_t out_b = (src_b * src_a + dst_b * (255 - src_a)) / 255;
        return (out_a << 24) | (out_r << 16) | (out_g << 8) | out_b;
}

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
        {
                uint32_t *dst = (uint32_t *)Dest->back;
                uint32_t src = Colour.rgba_888_w;
                uint8_t alpha = (src >> 24) & 0xFF;
                if (alpha == 255)
                    dst[x + rowOff] = src;
                else if (alpha > 0)
                    dst[x + rowOff] = blend_pixel(dst[x + rowOff], src);
                break;
        }
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

void AWM_DrawLine(AWM_Surface *Dest, AWM_Colour Colour, size_t X1, size_t Y1, size_t X2, size_t Y2)
{
        if (!Dest || !Dest -> back)
                return;
        int64_t x0 = X1;
        int64_t y0 = Y1;
        int64_t x1 = X2;
        int64_t y1 = Y2;

        size_t bpp = Dest -> bpp / 8;
        if (bpp == 0) return;

        int64_t width = Dest -> rect.w;
        int64_t height = Dest -> rect.h;

        int64_t dx = llabs(x1 - x0);
        int64_t dy = llabs(y1 - y0);
        int64_t sx = (x0 < x1) ? 1 : -1;
        int64_t sy = (y0 < y1) ? 1 : -1;
        int64_t err = dx - dy;

        while (1)
        {
                if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
                {
                        uint8_t *pixel = (uint8_t *) Dest -> back;
                        pixel += (y0 * width + x0) * bpp;
                        switch (Dest -> bpp)
                        {
                        case 32: // RGBA8888
                                *(uint32_t *) pixel = Colour.rgba_888_w;
                                break;
                        case 24: // RGB888
                                pixel[0] = (Colour.rgba_888_w >> 16) & 0xFF;
                                pixel[1] = (Colour.rgba_888_w >> 8) & 0xFF;
                                pixel[2] =  Colour.rgba_888_w & 0xFF;
                                break;
                        case 16: // RGB565
                        {
                                *(uint16_t *)pixel = Colour.rgba_555_w;
                        }
                        break;
                        case 8:
                                *pixel = Colour.palette_256_w & 0xFF;
                                break;
                        default:
                                return;
                        }
                }

                if (x0 == x1 && y0 == y1)
                        break;

                int64_t e2 = 2 * err;
                if (e2 > -dy)
                {
                        err -= dy;
                        x0 += sx;
                }

                if (e2 < dx)
                {
                        err += dx;
                        y0 += sy;
                }
        }
}

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
                end_row = start_row + ((int64_t)Dest->rect.h - dest_start_y);
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
                        copy_width = Dest->rect.w - dest_start_x;
                if (copy_width == 0)
                        continue;
                
                uint8_t *src_ptr = (uint8_t*)Surface->front;
                src_ptr += (row * src_stride) + (src_x * bytes_per_pixel);
                uint8_t *dst_ptr = (uint8_t*)Dest->back;
                dst_ptr += (dest_start_y + (row - start_row)) * dst_stride + (dest_start_x * bytes_per_pixel);
                if (Dest->bpp == 32)
                {
                        uint32_t *src_pixels = (uint32_t*)src_ptr;
                        uint32_t *dst_pixels = (uint32_t*)dst_ptr;
                        for (size_t i = 0; i < copy_width; i++)
                                dst_pixels[i] = blend_pixel(dst_pixels[i], src_pixels[i]);
                } else
                        memcpy(dst_ptr, src_ptr, copy_width * bytes_per_pixel);
        }
}

AWM_Surface AWM_LoadImage(const char *filename)
{
        int w, h, channels;
        unsigned char *data = stbi_load(filename, &w, &h, &channels, 4);
        if (!data)
        {
                return (AWM_Surface){0};
        }

        AWM_Surface surface = AWM_NewSurface(w, h, 32);
        uint32_t *dst = (uint32_t*)surface.back;
        for (int i = 0; i < w * h; i++)
        {
                uint8_t r = data[i*4 + 0];
                uint8_t g = data[i*4 + 1];
                uint8_t b = data[i*4 + 2];
                uint8_t a = data[i*4 + 3];
                dst[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        
        stbi_image_free(data);
        return surface;
}
