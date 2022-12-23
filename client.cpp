
#include "xpipc.h"
#include "snex.h"
#include <cstdio>

int main(void) {
    struct xpipc_shm shm;
    if (!xpipc_shm_open("/snex-ppu", sizeof(struct snex_shared), &shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    struct xpipc_event ev_server_ready;
    struct xpipc_event ev_client_ready;

    if (!xpipc_event_open("/snex-server-ready", &ev_server_ready)) {
        printf("%s\n", ev_server_ready.last_error);
        return 1;
    }

    if (!xpipc_event_open("/snex-client-ready", &ev_client_ready)) {
        printf("%s\n", ev_client_ready.last_error);
        return 1;
    }

    struct snex_shared *snex = (struct snex_shared *) shm.mapped;
    if (snex->version != 1) {
        printf("error; snex version != 1; got %d\n", snex->version);
        return 1;
    }

    for (;;) {
        // wait for NMI syn:
        if (!xpipc_event_wait(&ev_server_ready, 17)) {
            printf("wait: %s\n", ev_server_ready.last_error);
            continue;
        }

        printf("recv NMI syn\n");

        // ack:
        if (!xpipc_event_set(&ev_client_ready)) {
            printf("set:  %s\n", ev_client_ready.last_error);
            continue;
        }
        printf("send NMI ack\n");
    }

    printf("done\n");
    return 0;
}
