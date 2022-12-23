
#include "xpipc.h"
#include "snex.h"
#include <cstdio>

int main() {
    struct xpipc_shm shm;

    if (!xpipc_shm_create("/snex-ppu", sizeof(struct snex_shared), &shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    struct xpipc_event ev_server_ready;
    struct xpipc_event ev_client_ready;

    if (!xpipc_event_create("/snex-server-ready", &ev_server_ready)) {
        printf("%s\n", ev_server_ready.last_error);
        return 1;
    }

    if (!xpipc_event_create("/snex-client-ready", &ev_client_ready)) {
        printf("%s\n", ev_client_ready.last_error);
        return 1;
    }

    struct snex_shared *snex = (struct snex_shared *)shm.mapped;
    snex->version = 1;
    snex->v1.sync_state = 0;

    for (;;) {
        // switch to state 1 NMI-sync:
        printf("send NMI syn\n");
        if (!xpipc_event_set(&ev_server_ready)) {
            printf("set:  %s\n", ev_server_ready.last_error);
            continue;
        }

        // expect NMI acknowledged:
        if (!xpipc_event_wait(&ev_client_ready, 14)) {
            printf("wait: %s\n", ev_client_ready.last_error);
            continue;
        }

        // reset state to IDLE:
        printf("present\n");
        usleep(14000);
    }

    if (!xpipc_shm_close(&shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    return 0;
}
