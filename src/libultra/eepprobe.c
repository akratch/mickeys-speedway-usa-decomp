/*
 * Mickey's 0xB0-byte EEPROM probe follows the official SDK's 16-Kbit-aware
 * control flow. The placeholder name is retained so existing callers preserve
 * their authenticated relocation target.
 *
 * PROVENANCE: adapted from official libultra source published in the Banjo-
 * Kazooie decomp at `lib/ultralib/src/io/conteepprobe.c`, pinned in
 * `tools/reference-builds.lock`. This is a permitted published decomp source
 * under docs/CLEANROOM.md.
 */

#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

#define MICKEY_CONT_EEPROM 0x8000
#define MICKEY_CONT_EEP16K 0x4000
#define MICKEY_CONT_EEPROM_TYPE_MASK 0xF000
#define MICKEY_EEPROM_TYPE_4K 1
#define MICKEY_EEPROM_TYPE_16K 2

extern s32 __osEepStatus(OSMesgQueue *mq, OSContStatus *data);

s32 func_80070170(OSMesgQueue *mq) {
    s32 ret = 0;
    u16 type;
    OSContStatus data;

    __osSiGetAccess();
    ret = __osEepStatus(mq, &data);
    type = data.type & MICKEY_CONT_EEPROM_TYPE_MASK;

    if (ret != 0) {
        ret = 0;
    } else {
        switch (type) {
            case MICKEY_CONT_EEPROM:
                ret = MICKEY_EEPROM_TYPE_4K;
                break;
            case MICKEY_CONT_EEPROM | MICKEY_CONT_EEP16K:
            case MICKEY_CONT_EEPROM | 0x1000:
            case MICKEY_CONT_EEPROM | 0x2000:
                ret = MICKEY_EEPROM_TYPE_16K;
                break;
            default:
                ret = 0;
                break;
        }
    }

    __osSiRelAccess();
    return ret;
}
