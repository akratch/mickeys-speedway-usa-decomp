/*
 * libultra Transfer Pak connector check.
 *
 * Perfect Dark's built `lib/ultra/io/gbpakcheckconnector.o` is the unique
 * whole-object correspondence for Mickey's 0x490-byte translation unit. The
 * object belongs to its rolled-loop -O2 -mips2 -32 flag group.
 *
 * DEVIATION FROM THE REFERENCE: the power and read/write calls retain Mickey's
 * current tier-D placeholders `func_8006AC60` and `func_8006B020`. The target
 * TUs are not themselves known-object matches, so their SDK names are not yet
 * adopted here.
 *
 * PROVENANCE: adapted from the official libultra body published in the
 * Banjo-Kazooie decomp at `lib/ultralib/src/io/gbpakcheckconnector.c`;
 * Perfect Dark's `src/lib/ultra/io/gbpakcheckconnector.c` and built object
 * authenticate the TU identity and flag group. Both are permitted published
 * decomp sources under docs/CLEANROOM.md. Mickey's bytes decide the local
 * spelling, flags, and call targets.
 */

#include "PR/os_gbpak.h"
#include "PRinternal/controller.h"

s32 func_8006AC60(OSPfs *pfs, s32 flag);
s32 func_8006B020(OSPfs *pfs, u16 flag, u16 address, u8 *buffer, u16 size);

s32 osGbpakCheckConnector(OSPfs *pfs, u8 *status) {
    s32 ret;
    s32 bufn = 1;
    s32 oldbufn = 0;
    u16 address = 0;
    u16 oldaddr = 0;
    u16 daddr = 0;
    u16 num;
    u8 buf[3][4][BLOCKSIZE];
    u8 buf_status[3][4];

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

        bzero(buf_status, sizeof(buf_status));

        for (address = 0x80; address <= 0x4000; address <<= 1) {
            num = 0;
            daddr = 0;

            do {
                ERRCK(func_8006B020(pfs, PFS_READ, address + daddr,
                                    buf[bufn][num], BLOCKSIZE));
                buf_status[bufn][num] = 1;

                if (buf_status[oldbufn][num] == 0) {
                    ret = func_8006B020(pfs, PFS_READ, oldaddr + daddr,
                                        buf[oldbufn][num], BLOCKSIZE);

                    if (ret != 0) {
                        return ret;
                    } else {
                        buf_status[oldbufn][num] = 1;
                    }
                }

                if (bcmp(buf[bufn][num], buf[oldbufn][num], BLOCKSIZE) != 0) {
                    num = 0;
                    break;
                }

                daddr += BLOCKSIZE;
            } while (num++ < 3);

            if (num != 0) {
                return PFS_ERR_CONTRFAIL;
            }

            if (oldbufn != 0) {
                num = 0;
                daddr = 0;

                do {
                    if (buf_status[bufn][num] == 0) {
                        ERRCK(func_8006B020(pfs, PFS_READ, address + daddr,
                                            buf[bufn][num], BLOCKSIZE));
                        buf_status[bufn][num] = 1;
                    }

                    if (buf_status[0][num] == 0) {
                        ret = func_8006B020(pfs, PFS_READ, daddr,
                                            buf[0][num], BLOCKSIZE);
                        if (ret != 0) {
                            return ret;
                        } else {
                            buf_status[0][num] = 1;
                        }
                    }

                    if (bcmp(buf[bufn][num], buf[0][num], BLOCKSIZE)) {
                        num = 0;
                        break;
                    }

                    daddr += BLOCKSIZE;
                } while (num++ < 3);
            }

            if (num != 0) {
                return PFS_ERR_CONTRFAIL;
            }

            if (oldbufn != 0) {
                bzero(buf_status[oldbufn], sizeof(buf_status[oldbufn]));
            }

            oldaddr = address;
            oldbufn = bufn;
            bufn ^= 3;
        }

        if ((pfs->dir_size >= 2) || (pfs->version == 2)) {
            num = 0;
            daddr = 0;

            do {
                ERRCK(func_8006B020(pfs, PFS_READ, daddr + 0xA000,
                                    buf[bufn][num], BLOCKSIZE));
                ERRCK(func_8006B020(pfs, PFS_READ, daddr + 0x2000,
                                    buf[oldbufn][num], BLOCKSIZE));

                if (bcmp(buf[bufn][num], buf[oldbufn][num], BLOCKSIZE)) {
                    num = 0;
                    break;
                }

                daddr += BLOCKSIZE;
            } while (num++ < 3);

            if (num != 0) {
                return PFS_ERR_CONTRFAIL;
            }
        }

        ret = osGbpakGetStatus(pfs, status);

        if (ret == PFS_ERR_NEW_GBCART) {
            ret = PFS_ERR_CONTRFAIL;
        }
    }

    return ret;
}
