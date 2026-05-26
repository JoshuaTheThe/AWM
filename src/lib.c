
#include <lib.h>

void *AWM_New(size_t sizeof_T)
{
        void *m = mmap(NULL, sizeof_T, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        // manually for better errors
        if (m == MAP_FAILED)
        {
                panic(PANIC_NEW);
        }

        memset(m, 0, sizeof_T);
        return m;
}

void AWM_Drop(void *V, size_t sizeof_T)
{
        memset(V, 0, sizeof_T);
        // manually for better errors
        if (munmap(V, sizeof_T) == -1)
        {
                panic(PANIC_DROP);
        }
}

void AWM_Panic(size_t code)
{
        printf("Panic invoked with code: %lu\n", code);
        abort();
}

int AWM_Open(const char *const Path, size_t Flags)
{
        if (Path == NULL) return -1;
        int fd = open(Path, Flags);
        if (fd < 0)
                panic(PANIC_OPEN);
        return fd;
}

void AWM_Close(int fd)
{
        close(fd); // cleanup can go here, but is not required
}

void *AWM_MMap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
        void *p = mmap(addr, len, prot, flags, fd, offset);
        if (p == MAP_FAILED)
                panic(PANIC_MAP);
        return p;
}

void AWM_MUnMap(void *addr, size_t len)
{
        if (munmap(addr, len) < 0)
                panic(PANIC_UNMAP);
}
