#ifndef OVERLAYS_OVERLAY058_H
#define OVERLAYS_OVERLAY058_H

void overlay58RefreshRankSet(void);
void overlay58DrawSegmentStrip(float x0, float y0, float z0, float x1,
                               float y1, float z1, float limit);
void overlay58DrawPointQuad(s32 x, s32 y, s32 z);
void overlay58DrawLargePointQuad(s32 x, s32 y, s32 z);

#endif
