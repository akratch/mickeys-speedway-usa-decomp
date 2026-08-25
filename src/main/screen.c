/*
 * Compressed screen loading and drawing -- ROM 0x2F0D0-0x2F400.
 *
 * PROVENANCE -- the TU identity and names are adapted from Jet Force Gemini's
 * public decompilation, src/screen.c. Mickey's load/decompress and draw/VI
 * call graphs establish the two-function correspondence. The bodies remain
 * Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"

typedef struct ScreenCommand {
    u32 w0;
    u32 w1;
} ScreenCommand;

#define SCREEN_PRIMITIVE(command, color) \
    { \
        ScreenCommand *cmd = (command); \
        cmd->w0 = 0xFA000000; \
        cmd->w1 = (color); \
    }

#define SCREEN_FILL_RECT(command, ulx, uly, lrx, lry) \
    { \
        ScreenCommand *cmd = (command); \
        cmd->w0 = 0xF6000000 | (((lrx) & 0x3FF) << 14) | \
                  (((lry) & 0x3FF) << 2); \
        cmd->w1 = (((ulx) & 0x3FF) << 14) | (((uly) & 0x3FF) << 2); \
    }

#define SCREEN_PIPE_SYNC(command) \
    { \
        ScreenCommand *cmd = (command); \
        cmd->w0 = 0xE7000000; \
        cmd->w1 = 0; \
    }

/* Mickey-derived display-list command construction; JFG supplies the
 * existing screen TU/name correspondence, not this C body. */
void screenLoad(ScreenCommand **dlist, s32 ulx, s32 uly, s32 lrx, s32 lry,
                s32 drawOuter) {
    ScreenCommand *command;

    command = *dlist;
    if (drawOuter != 0) {
        SCREEN_PRIMITIVE(command++, 0);
        SCREEN_FILL_RECT(command++, ulx - 1, uly - 1, lrx + 1, lry + 1);
        SCREEN_PIPE_SYNC(command++);
    }
    SCREEN_PRIMITIVE(command++, 0xFF);
    SCREEN_FILL_RECT(command++, ulx, uly, lrx, lry);
    SCREEN_PIPE_SYNC(command++);
    *dlist = command;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/screen/screenDraw.s")
