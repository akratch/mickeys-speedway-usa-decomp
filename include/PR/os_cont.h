#ifndef _OS_CONT_H_
#define _OS_CONT_H_

/*
 * Controller query results and the joybus error bits the PFS code tests.
 *
 * `__osPfsGetOneChannelData` (ROM 0x6DD90) writes `errno` last at offset 0x4
 * and `type` at 0x0 with `sh`, and `__osPfsGetStatus` (ROM 0x6DBD0) tests
 * `status` with `andi 0x1` then `andi 0x2` then `andi 0x4`, which is what fixes
 * the three CONT_CARD/CRC bits below.
 *
 * PROVENANCE: names follow the N64 SDK header as published in public decomp
 * trees (JFG's `include/PR/os_cont.h`), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 1.3.
 */

#include "PR/ultratypes.h"

typedef struct {
    u16 type;
    u8 status;
    u8 errno;
} OSContStatus;

typedef struct {
    u16 button;
    s8 stick_x;
    s8 stick_y;
    u8 errno;
} OSContPad;

#define MAXCONTROLLERS 4

/* Joybus channel error codes, as they sit in the top nibble of `rxsize`. */
#define CONT_NO_RESPONSE_ERROR 0x8
#define CONT_OVERRUN_ERROR     0x4

/* Accessory status bits. */
#define CONT_CARD_ON     0x01
#define CONT_CARD_PULL   0x02
#define CONT_ADDR_CRC_ER 0x04

#endif /* _OS_CONT_H_ */
