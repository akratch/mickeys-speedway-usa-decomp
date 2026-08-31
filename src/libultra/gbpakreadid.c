/*
 * libultra Transfer Pak cartridge ID reader.
 *
 * Mickey's 0x2A4-byte function, in a 0x2B0-byte translation unit, contains
 * the VERSION_K+ reset/retry path from the SDK source. The signature and
 * cartridge-version tables already occupy authenticated resident data ranges,
 * so this TU references those objects without changing their ownership.
 *
 * DEVIATION FROM THE REFERENCE: the power and read/write calls retain Mickey's
 * current tier-D placeholders `func_8006AC60` and `func_8006B020`. Their TUs
 * have not yet earned the corresponding SDK names.
 *
 * PROVENANCE: adapted from the official libultra body published in the
 * Banjo-Kazooie decomp at `lib/ultralib/src/io/gbpakreadid.c`, selecting its
 * `BUILD_VERSION >= VERSION_K` path. This is a permitted published decomp
 * source under docs/CLEANROOM.md; Mickey's bytes decide the local data and
 * call identities.
 */

#include "PR/os_gbpak.h"
#include "PRinternal/controller.h"

extern u32 D_800803D0[];
extern u8 D_80080400[];

s32 func_8006AC60(OSPfs *pfs, s32 flag);
s32 func_8006B020(OSPfs *pfs, u16 flag, u16 address, u8 *buffer, u16 size);

s32 osGbpakReadId(OSPfs *pfs, OSGbpakId *id, u8 *status) {
    s32 i;
    s32 ret;
    u8 isum;
    u8 buf[96];
    u8 temp[32];

    ret = osGbpakGetStatus(pfs, status);

    if (ret == PFS_ERR_NEW_GBCART) {
        ret = osGbpakGetStatus(pfs, status);
    }

    if (ret == PFS_ERR_NEW_GBCART) {
        return PFS_ERR_CONTRFAIL;
    } else if (ret == 0) {
        if (!(*status & OS_GBPAK_POWER)) {
            ERRCK(func_8006AC60(pfs, OS_GBPAK_POWER_ON));
        }

        ERRCK(func_8006B020(pfs, PFS_READ, 0x100, buf, sizeof(buf)));

        ret = osGbpakGetStatus(pfs, status);

        if (ret == PFS_ERR_NEW_GBCART) {
            ret = PFS_ERR_CONTRFAIL;
        }

        if (ret != 0) {
            return ret;
        }

        if (!(*status & OS_GBPAK_RSTB_STATUS)) {
            return PFS_ERR_CONTRFAIL;
        }

        if (bcmp(D_800803D0, buf + 4, 0x30)) {
            for (i = 0; i < (s32)sizeof(temp); temp[i++] = 0) {
                ;
            }

            ERRCK(func_8006B020(pfs, PFS_WRITE, 0x6000, temp, sizeof(temp)));
            ret = func_8006B020(pfs, PFS_READ, 0x100, buf, sizeof(buf));
            ERRCK(func_8006B020(pfs, PFS_READ, 0x100, buf, sizeof(buf)));

            ret = osGbpakGetStatus(pfs, status);

            if (ret == PFS_ERR_NEW_GBCART) {
                ret = PFS_ERR_CONTRFAIL;
            }

            if (ret != 0) {
                return ret;
            }

            if (bcmp(D_800803D0, buf + 4, 0x30)) {
                return PFS_ERR_CONTRFAIL;
            }
        }

        for (i = 0x34, isum = 0; i < 0x4E; i++) {
            isum += buf[i];
        }

        if ((isum + 0x19) & 0xFF) {
            return PFS_ERR_CONTRFAIL;
        }

        bcopy(buf, id, sizeof(OSGbpakId));

        if (id->cart_type < 0x14) {
            pfs->version = (int)D_80080400[id->cart_type];
        }

        pfs->dir_size = (int)id->ram_size;
    }

    return ret;
}
