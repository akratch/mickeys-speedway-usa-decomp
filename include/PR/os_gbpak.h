#ifndef _OS_GBPAK_H_
#define _OS_GBPAK_H_

/*
 * Transfer Pak interface used by Mickey's matching libultra corridor.
 *
 * PROVENANCE: names and constants follow the SDK header published in the
 * Perfect Dark decompilation, a permitted source under docs/CLEANROOM.md.
 */

#include "PR/os_pfs.h"

typedef struct {
    u16 fixed1;
    u16 start_address;
    u8 nintendo_chr[0x30];
    u8 game_title[16];
    u16 company_code;
    u8 body_code;
    u8 cart_type;
    u8 rom_size;
    u8 ram_size;
    u8 country_code;
    u8 fixed2;
    u8 version;
    u8 isum;
    u16 sum;
} OSGbpakId;

#define OS_GBPAK_POWER          0x01
#define OS_GBPAK_RSTB_DETECTION 0x04
#define OS_GBPAK_RSTB_STATUS    0x08
#define OS_GBPAK_GBCART_PULL    0x40
#define OS_GBPAK_GBCART_ON      0x80

#define OS_GBPAK_POWER_OFF 0x00
#define OS_GBPAK_POWER_ON  0x01

s32 osGbpakGetStatus(OSPfs *pfs, u8 *status);
s32 osGbpakCheckConnector(OSPfs *pfs, u8 *status);
s32 osGbpakReadId(OSPfs *pfs, OSGbpakId *id, u8 *status);
s32 __osGbpakSelectBank(OSPfs *pfs, u8 bank);

#endif /* _OS_GBPAK_H_ */
