#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101PanelRect {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    u32 color;
} Overlay101PanelRect;

typedef struct Overlay101Panel {
    u8 pad00[8];
    u8 mode;
    s8 phase;
    s16 currentX;
    s16 currentY;
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    u8 intensity;
    u8 alpha;
    void *content;
} Overlay101Panel;

typedef struct Overlay101PanelWork {
    Overlay101PanelRect records[20];
    u32 padF0;
    u32 darkest;
    volatile u32 dim;
    volatile u32 dimmer;
    u32 full;
    s32 height;
    s32 width;
} Overlay101PanelWork;

extern u8 D_1F4[];

void func_overlay_101_F0000000_18DB820();

#define overlay101SetScissor func_overlay_101_F0000000_18DB820
#define func_80039584 func_overlay_101_F0000000_18DB820
#define overlay101BuildIntensityColors func_overlay_101_F0000000_18DB820
#define overlay101BuilderCreateReloc func_overlay_101_F0000000_18DB820
#define overlay101BuildBorder func_overlay_101_F0000000_18DB820
#define overlay101SelectElementReloc func_overlay_101_F0000000_18DB820
#define overlay101SetRenderModeReloc func_overlay_101_F0000000_18DB820
#define overlay101SetColorReloc func_overlay_101_F0000000_18DB820
#define overlay101DrawElementReloc func_overlay_101_F0000000_18DB820
#define overlay101DrawDefaultAssetReloc func_overlay_101_F0000000_18DB820

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor. */
void overlay101DrawPanel(Overlay101Gfx **displayList, Overlay101Panel *panel) {
    Overlay101PanelRect *out;
    s32 x;
    Overlay101PanelWork work;
    s32 y;
    s32 right;
    s32 bottom;

    overlay101SetScissor(displayList, 0, 0, 1000, 1000);

    if ((panel->mode == 1) || (panel->mode == 3)) {
        func_80039584(
            displayList,
            panel->currentX +
                (((panel->x - panel->currentX) * panel->phase) >> 6),
            panel->currentY +
                (((panel->y - panel->currentY) * panel->phase) >> 6),
            32 + (((panel->width - 32) * panel->phase) >> 6),
            32 + (((panel->height - 32) * panel->phase) >> 6),
            0xC0C0C0FF);
        return;
    }

    if (panel->mode == 2) {
        x = panel->x;
        y = panel->y;
        work.width = panel->width;
        work.height = panel->height;

        overlay101BuildIntensityColors(panel->intensity + 1, panel->alpha,
                                       &work.full, &work.dim, &work.dimmer,
                                       &work.darkest);

        right = x + work.width;
        bottom = y + work.height;
        out = work.records;

        out->x0 = x + 1;
        out->y0 = y + 12;
        out->x1 = x + 3;
        out->y1 = bottom - 1;
        out->color = work.dimmer;
        out++;

        out->x0 = x + 1;
        out->y0 = bottom - 3;
        out->x1 = right - 1;
        out->y1 = bottom - 1;
        out->color = work.dimmer;
        out++;

        out->x0 = right - 3;
        out->y0 = y + 12;
        out->x1 = right - 1;
        out->y1 = bottom - 1;
        out->color = work.dimmer;
        out++;

        out->x0 = x;
        out->y0 = y + 12;
        out->x1 = x + 1;
        out->y1 = bottom;
        out->color = work.dim;
        out++;

        out->x0 = x + 1;
        out->y0 = bottom - 1;
        out->x1 = right;
        out->y1 = bottom;
        out->color = work.dim;
        out++;

        out->x0 = right - 4;
        out->y0 = y + 12;
        out->x1 = right - 3;
        out->y1 = bottom - 3;
        out->color = work.dim;
        out++;

        out->x0 = x + 3;
        out->y0 = y + 12;
        out->x1 = x + 4;
        out->y1 = bottom - 3;
        out->color = work.full;
        out++;

        out->x0 = x + 4;
        out->y0 = bottom - 4;
        out->x1 = right - 3;
        out->y1 = bottom - 3;
        out->color = work.full;
        out++;

        out->x0 = right - 1;
        out->y0 = y + 12;
        out->x1 = right;
        out->y1 = bottom;
        out->color = work.full;
        out++;

        out->x0 = x + 4;
        out->y0 = y + 12;
        out->x1 = right - 4;
        out->y1 = bottom - 4;
        out->color = work.darkest;

        overlay101BuilderCreateReloc(displayList, 10, work.records, 0);

        overlay101BuildBorder(displayList, x, y, 12, 12,
                              panel->intensity, panel->alpha, 0);
        overlay101BuildBorder(displayList, x + 12, y, work.width - 36, 12,
                              panel->intensity, panel->alpha, 0);
        overlay101BuildBorder(displayList, right - 24, y, 12, 12,
                              panel->intensity, panel->alpha, 0);
        overlay101BuildBorder(displayList, right - 12, y, 12, 12,
                              panel->intensity, panel->alpha, 0);

        if (panel->content != NULL) {
            overlay101SetScissor(displayList, x + 14, y + 2, right - 26,
                                  y + 10);
            overlay101SelectElementReloc(2);
            overlay101SetRenderModeReloc(0, 0, 0, 0);
            overlay101SetColorReloc(0, 0, 0, 255, panel->alpha);
            overlay101DrawElementReloc(displayList, x + 14, y + 2,
                                       panel->content, 0);
            overlay101SetScissor(displayList, 0, 0, 1000, 1000);
        }
        return;
    }

    if (panel->mode == 0) {
        overlay101DrawDefaultAssetReloc(displayList, D_1F4,
                                        panel->currentX, panel->currentY,
                                        255, 255, 255, panel->alpha);
    }
}
