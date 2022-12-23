
#ifndef SNEX_SNEX_H
#define SNEX_SNEX_H

#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif

const int snex_ppu_screen_size = 1024*1024;
struct snex_ppu_screen {
    // 15-bit BGR color, MSB = 1 to override
    uint16_t color[snex_ppu_screen_size];
    // 00pp olll
    //   || ||||
    //   || |\++-- layer      (0=BG1, 1=BG2, 2=BG3, 3=BG4, 4=OAM, 5..7=undefined)
    //   || \----- under/over (0 = render underneath equivalent PPU layer/priority, 1 = render over equivalent PPU layer/priority)
    //   \+------- priority   (0..1 for BG1-3, 0..3 for OBJ)
    uint8_t  attrs[snex_ppu_screen_size];
};

struct snex_shared_v1 {
    _Atomic(int32_t)        sync_state;
    struct snex_ppu_screen  ppu_main;
    struct snex_ppu_screen  ppu_sub;
};

struct snex_shared {
    int32_t                 version;

    struct snex_shared_v1   v1;
};

#endif //SNEX_SNEX_H
