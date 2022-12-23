
#include "xpipc.h"
#include <cstdio>

static void xpipc_set_last_error(char *last_error) {
#if XPIPC_WINDOWS
    sprintf(last_error, "err %08lx", GetLastError());
#elif XPIPC_UNIX
    strerror_r(errno, last_error, 4096);
#endif
}

static void xpipc_clear_last_error(char *last_error) {
    last_error[0] = 0;
}

// takes a POSIX standard name like "/file" and strips the leading / off for Windows.
static void xpipc_set_name(const char *name, char *dest) {
#if XPIPC_WINDOWS
    dest[0] = 0;
    if (name[0] == '/') {
        //strcpy(shm->name, "Global\\");
        strncat(dest, name+1, 255);
    } else {
        strncpy(dest, name, 255);
    }
    //printf("%s\n", dest);
#else
    strncpy(shm->name, name, 255);
#endif
}

bool xpipc_shm_create(const char *name, size_t size, struct xpipc_shm *shm) {
#if XPIPC_WINDOWS
    xpipc_set_name(name, shm->name);
    shm->size = size;
    shm->hMapFile = NULL;
    shm->mapped = NULL;

    xpipc_clear_last_error(shm->last_error);
    shm->hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        shm->size >> 32,
        shm->size & 0xFFFFFFFF,
        shm->name
    );

    if (shm->hMapFile == NULL) {
        xpipc_set_last_error(shm->last_error);
        return false;
    }

    shm->mapped = MapViewOfFile(
        shm->hMapFile,   // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0,
        0,
        shm->size
    );
    if (shm->mapped == NULL) {
        xpipc_set_last_error(shm->last_error);

        CloseHandle(shm->hMapFile);

        shm->hMapFile = NULL;
        return false;
    }

    return true;
#elif XPIPC_UNIX
    xpipc_set_name(name, shm->name);
    shm->size = size;
    shm->fd = -1;
    shm->mapped = NULL;

    xpipc_clear_last_error(shm->last_error);
    shm->fd = shm_open(shm->name, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (shm->fd < 0) {
        xpipc_set_last_error(shm->last_error);
        return false;
    }

    if (ftruncate(shm->fd, shm->size) < 0) {
        xpipc_set_last_error(shm->last_error);
        return false;
    }

    shm->mapped = mmap(
        NULL,
        shm->size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm->fd,
        0
    );
    if (shm->mapped == MAP_FAILED) {
        xpipc_set_last_error(shm->last_error);
        shm->mapped = NULL;
        close(shm->fd);
        shm->fd = -1;
        return false;
    }

    return true;
#else
#  error Unsupported platform
#endif
}

bool xpipc_shm_open(const char *name, size_t size, struct xpipc_shm *shm) {
#if XPIPC_WINDOWS
    xpipc_set_name(name, shm->name);
    shm->size = size;
    shm->hMapFile = NULL;
    shm->mapped = NULL;

    xpipc_clear_last_error(shm->last_error);
    shm->hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        shm->name
    );

    if (shm->hMapFile == NULL) {
        xpipc_set_last_error(shm->last_error);
        return false;
    }

    shm->mapped = MapViewOfFile(
        shm->hMapFile,   // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0,
        0,
        shm->size
    );
    if (shm->mapped == NULL) {
        xpipc_set_last_error(shm->last_error);

        CloseHandle(shm->hMapFile);

        shm->hMapFile = NULL;
        return false;
    }

    return true;
#elif XPIPC_UNIX
    xpipc_set_name(name, shm->name);
    shm->size = size;
    shm->fd = -1;
    shm->mapped = NULL;

    xpipc_clear_last_error(shm->last_error);
    shm->fd = shm_open(shm->name, O_RDWR, 0666);
    if (shm->fd < 0) {
        xpipc_set_last_error(shm->last_error);
        return false;
    }

    shm->mapped = mmap(
        NULL,
        shm->size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm->fd,
        0
    );
    if (shm->mapped == MAP_FAILED) {
        xpipc_set_last_error(shm->last_error);
        shm->mapped = NULL;
        close(shm->fd);
        shm->fd = -1;
        return false;
    }

    return true;
#else
#  error Unsupported platform
#endif
}

bool xpipc_shm_close(struct xpipc_shm *shm) {
#if XPIPC_WINDOWS
    xpipc_clear_last_error(shm->last_error);
    if (!UnmapViewOfFile(shm->mapped)) {
        xpipc_set_last_error(shm->last_error);
        return false;
    }
    shm->mapped = NULL;

    if (!CloseHandle(shm->hMapFile)) {
        xpipc_set_last_error(shm->last_error);
        return false;
    }
    shm->hMapFile = NULL;

    return true;
#elif XPIPC_UNIX
    xpipc_clear_last_error(ev->last_error);
    if (munmap(shm->mapped, shm->size)) {
        xpipc_set_last_error(shm->last_error);
        return false;
    }
    shm->mapped = NULL;

    close(shm->fd);
    shm->fd = -1;

    shm_unlink(shm->name);

    return true;
#endif
}

bool xpipc_event_create(const char *name, struct xpipc_event *ev) {
#if XPIPC_WINDOWS
    xpipc_set_name(name, ev->name);

    xpipc_clear_last_error(ev->last_error);
    ev->hEvent = CreateEvent(
        NULL,
        FALSE,
        FALSE,
        ev->name
    );
    if (ev->hEvent == NULL) {
        xpipc_set_last_error(ev->last_error);
        return false;
    }

    return true;
#elif XPIPC_UNIX
#endif
}

bool xpipc_event_open(const char *name, struct xpipc_event *ev) {
#if XPIPC_WINDOWS
    xpipc_set_name(name, ev->name);

    xpipc_clear_last_error(ev->last_error);
    ev->hEvent = OpenEvent(
        SYNCHRONIZE | EVENT_MODIFY_STATE,
        FALSE,
        ev->name
    );
    if (ev->hEvent == NULL) {
        xpipc_set_last_error(ev->last_error);
        return false;
    }

    return true;
#elif XPIPC_UNIX
#endif
}

bool xpipc_event_set(struct xpipc_event *ev) {
#if XPIPC_WINDOWS
    xpipc_clear_last_error(ev->last_error);
    if (!SetEvent(ev->hEvent)) {
        xpipc_set_last_error(ev->last_error);
        return false;
    }

    return true;
#endif
}

bool xpipc_event_reset(struct xpipc_event *ev) {
#if XPIPC_WINDOWS
    xpipc_clear_last_error(ev->last_error);
    if (!ResetEvent(ev->hEvent)) {
        xpipc_set_last_error(ev->last_error);
        return false;
    }

    return true;
#endif
}

bool xpipc_event_wait(struct xpipc_event *ev, unsigned long millis) {
#if XPIPC_WINDOWS
    xpipc_clear_last_error(ev->last_error);
    DWORD result = WaitForSingleObject(ev->hEvent, millis);
    if (result == WAIT_OBJECT_0) {
        return true;
    }

    if (result == WAIT_FAILED) {
        xpipc_set_last_error(ev->last_error);
        return false;
    }
    if (result == WAIT_TIMEOUT) {
        strcpy(ev->last_error, "timeout");
        return false;
    }
    if (result == WAIT_ABANDONED_0) {
        strcpy(ev->last_error, "abandoned");
        return false;
    }

    return false;
#endif
}

bool xpipc_event_close(const char *name, struct xpipc_event *ev) {
#if XPIPC_WINDOWS
    xpipc_clear_last_error(ev->last_error);
    if (!CloseHandle(ev->hEvent)) {
        xpipc_set_last_error(ev->last_error);
        return false;
    }
    ev->hEvent = NULL;

    return true;
#endif
}