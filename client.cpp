
#include "xpipc.h"
#include "snex.h"
#include <cstdio>

int main(void) {
    struct xpipc_shm shm;
    if (!xpipc_shm_open("/snex-ppu", sizeof(struct snex_shared), &shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    struct snex_shared *snex = (struct snex_shared *)shm.mapped;
    if (snex->version != 1) {
        printf("error; snex version != 1; got %d\n", snex->version);
        return 1;
    }


    snex->v1.ppu_ready.store(1);
    printf("set screen ready\n");


    printf("done\n");
    return 0;
}
