#include "PR/ultratypes.h"

typedef struct Overlay40FrameRecord {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
    u32 color;
} Overlay40FrameRecord;

extern void overlay40SubmitFrameReloc(void *displayList, s32 count,
                                      Overlay40FrameRecord *records,
                                      s32 flags);

#line 22
void overlay40BuildFrame(void *displayList, s32 x, s32 y, s32 width,
                         s32 height, s32 red, s32 green, s32 blue, s32 alpha) {
#line 26
    struct {
        u64 pad;
        Overlay40FrameRecord records[8];
    } frame;
#line 34
    s32 right;
    s32 bottom;
    s32 bottomPlus2;
#line 40
    u32 color;

#line 72
    right = x + width;
    bottom = y + height;
    bottomPlus2 = bottom + 2;
#line 78
    color = (red << 24) | (green << 16) | (blue << 8) | (alpha & 0xFF);

#line 95
    frame.records[0].left = x - 2;
    frame.records[0].top = y - 2;
    frame.records[0].right = right + 2;
    frame.records[0].bottom = y + 3;
    frame.records[0].color = 0;

#line 101
    frame.records[1].left = x;
    frame.records[1].top = y;
    frame.records[1].right = right + 1;
    frame.records[1].bottom = y + 1;
    frame.records[1].color = color;

#line 107
    frame.records[2].left = x - 2;
    frame.records[2].top = y - 2;
    frame.records[2].right = x + 3;
    frame.records[2].bottom = bottomPlus2;
    frame.records[2].color = 0;

#line 113
    frame.records[3].left = x;
    frame.records[3].top = y + 1;
    frame.records[3].right = x + 1;
    frame.records[3].bottom = bottom;
    frame.records[3].color = color;

#line 119
    frame.records[4].left = right - 2;
    frame.records[4].top = y - 2;
    frame.records[4].right = right + 3;
#line 125
    frame.records[4].bottom = *(volatile s32 *)&bottomPlus2;
#line 129
    frame.records[4].color = 0;

#line 131
    frame.records[5].left = right;
    frame.records[5].top = y + 1;
    frame.records[5].right = right + 1;
    frame.records[5].bottom = bottom;
    frame.records[5].color = color;

#line 137
    frame.records[6].left = x - 2;
    frame.records[6].top = bottom - 2;
    frame.records[6].right = right + 2;
    frame.records[6].bottom = bottom + 3;
    frame.records[6].color = 0;

#line 143
    frame.records[7].left = x;
    frame.records[7].top = bottom;
    frame.records[7].right = right + 1;
    frame.records[7].bottom = bottom + 1;
    frame.records[7].color = color;

#line 149
    overlay40SubmitFrameReloc(displayList, 8, frame.records, 0);
}
