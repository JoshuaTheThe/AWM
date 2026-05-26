
// custom library functions

#ifndef LIB_H
#define LIB_H

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>
#include<linux/fb.h>
#include<sys/ioctl.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>

#define new(T) ((T *)AWM_New(sizeof(T)))
#define drop(V) AWM_Drop(V, sizeof(typeof(*V)))
#define panic(x) AWM_Panic(x)

enum
{
        PANIC_UNKNOWN,
        PANIC_NEW,
        PANIC_DROP,
        PANIC_DEBUG,
        PANIC_TODO,
        PANIC_OPEN,
        PANIC_NULL,
        PANIC_MAP,
        PANIC_UNMAP,
        PANIC_VINFO,
        PANIC_FILE_NOT_FOUND,
};

void *AWM_New(size_t sizeof_T);
void  AWM_Drop(void *V, size_t sizeof_T);
void  AWM_Panic(size_t code);
int   AWM_Open(const char *const Path, size_t Flags);
void  AWM_Close(int fd);
void *AWM_MMap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void  AWM_MUnMap(void *addr, size_t len);

#endif

