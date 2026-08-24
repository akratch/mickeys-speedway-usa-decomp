#include "PR/ultratypes.h"

extern char *gOverlay61FileNames[];
extern u8 gOverlay61FileStatus[];

/* DKR save_data.c has the same controller-pak extension-selection semantics,
 * but pinned DKR v77/v80 and JFG contain no exact object donor. */
void overlay61ChooseFileExtension(char *output) {
    s32 i;
    u8 used[26];

    for (i = 0; i < 26; i++) {
        used[i] = 0;
    }

    for (i = 0; i < 16; i++) {
        if (gOverlay61FileStatus[i] == 0) {
            if ((gOverlay61FileNames[i][0] >= 'A') &&
                (gOverlay61FileNames[i][0] <= 'Z')) {
                used[gOverlay61FileNames[i][0] - 'A'] = 1;
            }
        }
    }

    i = 0;
    while (used[i] != 0) {
        i++;
    }
    output[0] = i + 'A';
    output[1] = '\0';
}
