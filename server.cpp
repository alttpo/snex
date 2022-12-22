#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdatomic.h>
#include <cstdio>

const int snex_ppu_screen_size = 1024*1024;
struct snex_ppu_screen {
    // 15-bit BGR color, MSB = 1 to override
    uint16_t color[snex_ppu_screen_size];
    // 0000 ppll
    //      ||||
    //      ||\+-- layer    (0=BG1, 1=BG2, 2=BG3, 3=OBJ)
    //      \+---- priority (0..1 for BG1-3, 0..3 for OBJ)
    uint8_t  layer_prio[snex_ppu_screen_size];
};

struct snex_client {
    _Atomic int32_t         ppu_frames_ttl;
    struct snex_ppu_screen  ppu_main;
    struct snex_ppu_screen  ppu_sub;
};

struct snex_shared_v1 {
    _Atomic int32_t     last_client_idx;
    struct snex_client  clients[1];
};

struct snex_shared {
    int32_t                 version;

    struct snex_shared_v1   v1;
};

int main(void) {
    int fd = shm_open("/snex-ppu", O_RDWR | O_CREAT | O_EXCL, 0666);
    if (fd < 0) {
        perror("shm_open: ");
        return 1;
    }

    if (ftruncate(fd, sizeof(struct snex_shared)) < 0) {
        perror("ftruncate: ");
        return 1;
    }

    struct snex_shared *snex = (struct snex_shared *) mmap(
        NULL,
        sizeof(struct snex_shared),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );
    if (snex == MAP_FAILED) {
        perror("mmap: ");
        return 1;
    }
    if (close(fd) < 0) {
        perror("close");
        return 1;
    };

    snex->version = 1;
    snex->v1.last_client_idx = -1;

    for (;;) {

    }

    shm_unlink("/snex");
    return 0;
}
