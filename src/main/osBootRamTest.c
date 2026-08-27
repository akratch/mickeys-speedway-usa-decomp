/*
 * The CIC 6105 writes two test words near the end of RDRAM before the game
 * starts. These functions check and clear those words.
 *
 * PROVENANCE: bodies adapted from Jet Force Gemini's published
 * src/osBootRamTest.c at commit c82afff (the reference-build lock).
 * Mickey's configured object and linked ROM are verified independently.
 */

#include "PR/ultratypes.h"

#define READ_ADDRESS_DIRECT(address) (*(volatile u32 *)(address))
#define WRITE_ADDRESS_DIRECT(address, value) (*(volatile u32 *)(address) = (value))

/* PROVENANCE: body adapted from JFG src/osBootRamTest.c::osBootRamTest1_6105. */
s32 osBootRamTest1_6105(void) {
    s32 result;

    result = 0;
    if (READ_ADDRESS_DIRECT(0xA02FB1F4) == 0xAD090010) {
        result = 1;
    }
    WRITE_ADDRESS_DIRECT(0xA02FB1F4, 0);
    return result;
}

/* PROVENANCE: body adapted from JFG src/osBootRamTest.c::osBootRamTest2_6105. */
s32 osBootRamTest2_6105(void) {
    s32 result;

    result = 0;
    if (READ_ADDRESS_DIRECT(0xA02FE1C0) == 0xAD170014) {
        result = 1;
    }
    WRITE_ADDRESS_DIRECT(0xA02FE1C0, 0);
    return result;
}
