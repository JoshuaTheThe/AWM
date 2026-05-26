
#ifndef FONT_H
#define FONT_H

typedef struct __attribute__((__packed__))
{
        const unsigned char *font_name;
        unsigned char *font_bitmap;
        unsigned int char_width;
        unsigned int char_height;
        unsigned char first_char;
        unsigned char last_char;
        unsigned char bpp;
} AWM_Font;

AWM_Font *AWM_LoadFont(const char *path);
void      AWM_DestroyFont(AWM_Font *font);

#endif
