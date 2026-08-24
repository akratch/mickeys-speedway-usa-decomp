#ifndef _OS_GBPAK_H_
#define _OS_GBPAK_H_

/*
 * Transfer Pak interface used by Mickey's matching libultra corridor.
 *
 * PROVENANCE: names and constants follow the SDK header published in the
 * Perfect Dark decompilation, a permitted source under docs/CLEANROOM.md.
 */

#include "PR/os_pfs.h"

#define OS_GBPAK_POWER          0x01
#define OS_GBPAK_RSTB_DETECTION 0x04
#define OS_GBPAK_RSTB_STATUS    0x08
#define OS_GBPAK_GBCART_PULL    0x40
#define OS_GBPAK_GBCART_ON      0x80

#define OS_GBPAK_POWER_OFF 0x00
#define OS_GBPAK_POWER_ON  0x01

s32 osGbpakGetStatus(OSPfs *pfs, u8 *status);
s32 __osGbpakSelectBank(OSPfs *pfs, u8 bank);

#endif /* _OS_GBPAK_H_ */
