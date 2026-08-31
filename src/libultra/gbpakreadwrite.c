/*
 * libultra Transfer Pak block reader/writer.
 *
 * Mickey's 0x1CC-byte function, in a 0x1D0-byte translation unit, contains
 * the VERSION_K+ zero-size guard. Its calls bind to the already-proven local
 * bank selector and Controller Pak I/O functions.
 *
 * DEVIATION FROM THE REFERENCE: the definition retains Mickey's current
 * tier-D placeholder `func_8006B020` so the already-integrated callers keep
 * their relocation identity. The SDK name is not adopted piecemeal.
 *
 * PROVENANCE: adapted from the official libultra body published in the
 * Banjo-Kazooie decomp at `lib/ultralib/src/io/gbpakreadwrite.c`, selecting
 * its `BUILD_VERSION >= VERSION_K` path. This is a permitted published decomp
 * source under docs/CLEANROOM.md; Mickey's bytes decide the local spelling and
 * call identities.
 */

#include "PR/os_gbpak.h"
#include "PRinternal/controller.h"

s32 func_8006B020(OSPfs *pfs, u16 flag, u16 address, u8 *buffer, u16 size) {
    s32 i;
    s32 ret;
    u8 bank;

    bank = (u8)(address >> 0xE);

    if (bank != pfs->banks) {
        ret = __osGbpakSelectBank(pfs, bank);

        if (ret != 0) {
            return ret;
        }
    }

    if (size == 0) {
        return 0;
    }

    size = (u16)(size >> 5);
    address = (u16)((address | 0xC000) >> 5);

    if (flag == PFS_WRITE) {
        for (i = 0; i < (s32)size; i++, buffer += BLOCKSIZE) {
            ret = __osContRamWrite(pfs->queue, pfs->channel, address, buffer, 0);

            if (ret != 0) {
                break;
            }

            if ((++address >= 0x800) && (i < (s32)(size - 1))) {
                ret = __osGbpakSelectBank(pfs, ++bank);

                if (ret != 0) {
                    break;
                }

                address = 0x600;
            }
        }
    } else {
        for (i = 0; i < (s32)size; i++, buffer += BLOCKSIZE) {
            ret = __osContRamRead(pfs->queue, pfs->channel, address, buffer);

            if (ret != 0) {
                break;
            }

            if ((++address >= 0x800) && (i < (s32)(size - 1))) {
                ret = __osGbpakSelectBank(pfs, ++bank);

                if (ret != 0) {
                    break;
                }

                address = 0x600;
            }
        }
    }

    if (ret == PFS_ERR_NEW_PACK) {
        ret = PFS_ERR_CONTRFAIL;
    }

    return ret;
}
