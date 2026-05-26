
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <font.h>

AWM_Font *AWM_LoadFont(const char *path)
{
        AWM_Font *font = NULL;
        FILE *file = NULL;
        unsigned char *buffer = NULL;
        unsigned char *ptr = NULL;
        unsigned char *bitmap = NULL;
        unsigned char *font_name = NULL;
        long file_size;
        unsigned long version;
        unsigned long char_width, char_height;
        unsigned long first_char, last_char;
        unsigned long bpp, data_size;
        const char *name;
        unsigned int name_len;

        file = fopen(path, "rb");
        if (!file)
        {
                printf("Could not open font file: %s\n", path);
                abort();
                goto cleanup;
        }

        /* get file size */
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        buffer = (unsigned char *)malloc(file_size);
        if (!buffer)
        {
                printf("Could not allocate buffer for font\n");
                abort();
                goto cleanup;
        }

        if (fread(buffer, 1, file_size, file) != (unsigned int)file_size)
        {
                printf("Could not read font file\n");
                abort();
                goto cleanup;
        }

        ptr = buffer;

        /* check magic */
        if (memcmp(ptr, "FONT", 4) != 0)
        {
                printf("Invalid font magic\n");
                abort();
                goto cleanup;
        }
        ptr += 4;

        /* read version */
        version = *(unsigned int *)ptr;
        ptr += 4;
        if (version != 1)
        {
                printf("Unsupported font version: %lu\n", version);
                abort();
                goto cleanup;
        }

        /* read properties */
        char_width  = *(unsigned int *)ptr; ptr += 4;
        char_height = *(unsigned int *)ptr; ptr += 4;
        first_char  = *(unsigned int *)ptr; ptr += 4;
        last_char   = *(unsigned int *)ptr; ptr += 4;
        bpp         = *(unsigned int *)ptr; ptr += 4;
        data_size   = *(unsigned int *)ptr; ptr += 4;

        /* read font name */
        name = (const char *)ptr;
        name_len = strlen(name);
        if (name_len > 255) name_len = 255;
        ptr += name_len + 1;

        if (data_size != (unsigned int)(file_size - (ptr - buffer)))
        {
                printf("Font data size mismatch\n");
                abort();
                goto cleanup;
        }

        font = (AWM_Font *)malloc(sizeof(AWM_Font));
        if (!font)
        {
                printf("Could not allocate font structure\n");
                abort();
                goto cleanup;
        }

        bitmap = (unsigned char *)malloc(data_size);
        if (!bitmap)
        {
                printf("Could not allocate font bitmap\n");
                abort();
                free(font);
                font = NULL;
                goto cleanup;
        }
        memcpy(bitmap, ptr, data_size);

        font_name = (unsigned char *)malloc(name_len + 1);
        if (font_name)
                memcpy(font_name, name, name_len + 1);
        else
                font_name = (unsigned char *)"Unknown";

        font->font_name   = font_name;
        font->font_bitmap = bitmap;
        font->char_width  = (unsigned int)char_width;
        font->char_height = (unsigned int)char_height;
        font->first_char  = (unsigned char)first_char;
        font->last_char   = (unsigned char)last_char;
        font->bpp         = (unsigned char)bpp;
cleanup:
        if (file)   fclose(file);
        if (buffer) free(buffer);
        return font;
}

void AWM_DestroyFont(AWM_Font *font)
{
        if (!font)
                return;
        if (font->font_name && !strncmp(font->font_name, "Unknown", 8))
        {
                free((void *)font->font_name);
        }
        if (font->font_bitmap)
        {
                free(font->font_bitmap);
        }
        free(font);
}

