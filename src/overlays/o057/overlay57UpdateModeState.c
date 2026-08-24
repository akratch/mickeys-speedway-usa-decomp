#include "PR/ultratypes.h"

typedef struct Overlay57SetupObject {
    u8 reserved00[0x0C];
    s16 x0C;
    s16 y0E;
} Overlay57SetupObject;

typedef struct Overlay57MarkedEntry {
    s16 value00;
    u8 reserved02[2];
    u32 flags04;
} Overlay57MarkedEntry;

typedef struct Overlay57CountOwner {
    u8 reserved00[0x2C];
    u8 count2C;
} Overlay57CountOwner;

typedef struct Overlay57Selection {
    Overlay57CountOwner *countOwner00;
    u8 reserved04[0x48];
    Overlay57MarkedEntry *entries4C;
} Overlay57Selection;

typedef struct Overlay57SelectionChild {
    u8 reserved00[0x3A];
    s8 selector3A;
    u8 reserved3B[0x2D];
    Overlay57Selection **selections68;
} Overlay57SelectionChild;

typedef struct Overlay57LookupResult {
    u8 reserved00[8];
    Overlay57SelectionChild *child08;
} Overlay57LookupResult;

#define O57_SUB_WRAP(left, right) ((s32)((u32)(left) - (u32)(right)))

extern u32 gOverlay57Flags3B6CReloc;
extern s32 gOverlay57Mode3B88Reloc;
extern u8 gOverlay57Byte3D1CReloc;
extern u8 gOverlay57Byte3EF4Reloc;

extern void overlay57SetNodeValue(s32 id, s32 argument, f32 value);
extern void overlay57UpdateNode(void);
extern void overlay57ApplyTable(void);

extern s32 gO57ModeCountdown120;
extern s32 gO57ModeTimer114;
extern s32 gO57ModeState144BaseReloc[];
extern s32 gO57ModeState11CBaseReloc[];
extern s32 gO57ModeState118BaseReloc[];
extern s32 gO57ModeState160BaseReloc[];
extern s32 gO57ModeState164BaseReloc[];
extern s32 gO57ModeState184BaseReloc[];
extern s32 gO57ModeChoice4F8;
extern s32 gO57ModeGate50C[];
extern s32 gO57ModePrimaryIds134[];
extern s32 gO57ModeSecondaryIds1A0[];
extern Overlay57SetupObject gO57ModeSetup21C;
extern s32 gO57ModeTimerEarlyStoreBaseReloc[];
extern s32 gO57ModeTimerReloadBaseReloc[];
extern s32 gO57ModeTimerA5BaseReloc[];
extern s32 gO57ModeTimerA6BaseReloc[];
extern s32 gO57ModeTimerA7BaseReloc[];
extern s32 gO57ModeTimerB5BaseReloc[];
extern s32 gO57ModeTimerB6BaseReloc[];

#undef O57_S32
#define D_0x118 gO57ModeState118BaseReloc[0x118 / 4]
#define D_0x11C gO57ModeState11CBaseReloc[0x11C / 4]
#define D_0x120 gO57ModeCountdown120
#define D_0x144 gO57ModeState144BaseReloc[0x144 / 4]
#define D_0x160 gO57ModeState160BaseReloc[0x160 / 4]
#define D_0x164 gO57ModeState164BaseReloc[0x164 / 4]
#define D_0x184 gO57ModeState184BaseReloc[0x184 / 4]
#define D_0x4F8 gO57ModeChoice4F8
#define D_0x50C gO57ModeGate50C[0]
#define O57_JOIN_RAW(left, right) left##right
#define O57_JOIN(left, right) O57_JOIN_RAW(left, right)
#define O57_S32(offset) O57_JOIN(D_, offset)

extern s32 o57ModeOpaqueCallReloc();
extern void o57ModeOpaqueVoidCallReloc();
extern Overlay57LookupResult *o57ModeOpaquePtrCallReloc();
#define overlay57Call3AA4Reloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3AE0Reloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3B4CReloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3BC4Reloc o57ModeOpaqueCallReloc
#define overlay57Call3BECReloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3BF8Reloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3C08Reloc o57ModeOpaqueCallReloc
#define overlay57Call3C4CReloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3C5CReloc o57ModeOpaqueCallReloc
#define overlay57Call3C84Reloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3C94Reloc o57ModeOpaqueCallReloc
#define overlay57Call3CDCReloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3D08Reloc o57ModeOpaqueCallReloc
#define overlay57Call3D24Reloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3D34Reloc o57ModeOpaquePtrCallReloc
#define overlay57Call3DC4Reloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3DD4Reloc o57ModeOpaqueCallReloc
#define overlay57Call3DFCReloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3E08Reloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3E18Reloc o57ModeOpaqueCallReloc
#define overlay57Call3E5CReloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3E6CReloc o57ModeOpaqueCallReloc
#define overlay57Call3EB4Reloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3EE0Reloc o57ModeOpaqueCallReloc
#define overlay57Call3EFCReloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3F0CReloc o57ModeOpaquePtrCallReloc
#define overlay57Call3FA4Reloc o57ModeOpaqueVoidCallReloc
#define overlay57Call3FB8Reloc o57ModeOpaqueVoidCallReloc

void overlay57UpdateModeState(s32 updateRate) {
    s32 eligible;
    volatile s32 savedEligible;

    O57_S32(0x144) = 1;
    {
        s32 *list;
        u8 id;

        if (gO57ModeCountdown120 > 0) {
            if ((gO57ModeCountdown120 = O57_SUB_WRAP(gO57ModeCountdown120, updateRate)) > 0) {
                return;
            }

            list = gO57ModePrimaryIds134;
            while (*list != -1) {
                id = (u8)*list;
                overlay57Call3AA4Reloc(id);
                list++;
            }

            list = gO57ModeSecondaryIds1A0;
            while (*list != -1) {
                id = (u8)*list;
                overlay57Call3AE0Reloc(id);
                overlay57SetNodeValue(*list, 1, 0.0079f);
                list++;
            }

            gO57ModeTimerEarlyStoreBaseReloc[0x114 / 4] = 60;
            O57_S32(0x11C) = 0;
            return;
        }
    }

    {
        s32 timer;

        timer = gO57ModeTimer114;
        if ((timer >= 31) && (O57_SUB_WRAP(timer, updateRate) < 31)) {
            overlay57Call3B4CReloc(0x10, 0);
            timer = gO57ModeTimerReloadBaseReloc[0x114 / 4];
        }
        gO57ModeTimer114 = O57_SUB_WRAP(timer, updateRate);
    }

    eligible = (gOverlay57Flags3B6CReloc & 0xD000) != 0;
    if (eligible != 0) {
        eligible = gO57ModeGate50C[0] == 0;
    }
    savedEligible = eligible;
    if (gO57ModeTimer114 > 0) {
        if (eligible == 0) {
            return;
        }
    }

    if (gOverlay57Mode3B88Reloc != 1) {
        goto mode_b;
    }
        if (overlay57Call3BC4Reloc() == 8) {
            overlay57Call3BECReloc(0x2A, 0, 0, 0xE, 1, 0);
            gO57ModeGate50C[0] = 1;
            overlay57Call3BF8Reloc();
        } else if (overlay57Call3C08Reloc() == 5) {
            O57_S32(0x118) = 9;
            O57_S32(0x164) = 0;
            O57_S32(0x160) = 0;
            O57_S32(0x184) = 0x2F;
            gO57ModeTimerA5BaseReloc[0x114 / 4] = 0xB4;
            overlay57Call3C4CReloc(1);
        } else if (overlay57Call3C5CReloc() == 6) {
            O57_S32(0x118) = 12;
            gO57ModeTimerA6BaseReloc[0x114 / 4] = 0xB4;
            overlay57Call3C84Reloc(2);
        } else if (overlay57Call3C94Reloc() == 7) {
            O57_S32(0x118) = 21;
            O57_S32(0x160) = 0;
            O57_S32(0x164) = 0;
            O57_S32(0x184) = 0x4B;
            gO57ModeTimerA7BaseReloc[0x114 / 4] = 0xB4;
            overlay57Call3CDCReloc(1);
        } else {
            Overlay57LookupResult *result;
            Overlay57SelectionChild *child;
            Overlay57Selection *selection;
            Overlay57MarkedEntry *entries;
            s32 choice;
            s32 count;

            O57_S32(0x118) = 15;
            gO57ModeSetup21C.x0C = 0x17C;
            gO57ModeSetup21C.y0E = 0xBE;
            choice = overlay57Call3D08Reloc();
            gO57ModeChoice4F8 = choice;
            gOverlay57Byte3D1CReloc = (u8)choice;
            overlay57Call3D24Reloc(0x4D);
            overlay57UpdateNode();
            result = overlay57Call3D34Reloc(0x4C);
            if (result != 0) {
                child = result->child08;
                if (child != 0) {
                    child->selector3A = (s8)gO57ModeChoice4F8;
                    child = result->child08;
                    selection = child->selections68[child->selector3A];
                    if (selection != 0) {
                        entries = selection->entries4C;
                        if (entries != 0) {
                            count = selection->countOwner00->count2C;
                            if (count--) {
                                do {
                                    if ((entries->flags04 & 0x00100000) != 0) {
                                        entries->value00 = 0x100;
                                    }
                                    entries++;
                                } while (count--);
                            }
                        }
                    }
                }
            }
            overlay57Call3DC4Reloc(3);
        }
    goto dispatch_done;

mode_b:
        if (overlay57Call3DD4Reloc() == 8) {
            overlay57Call3DFCReloc(0x2A, 0, 0, 0xE, 1, 0);
            gO57ModeGate50C[0] = 1;
            overlay57Call3E08Reloc();
        } else if (overlay57Call3E18Reloc() == 5) {
            O57_S32(0x118) = 9;
            O57_S32(0x164) = 0;
            O57_S32(0x160) = 0;
            O57_S32(0x184) = 0x2F;
            gO57ModeTimerB5BaseReloc[0x114 / 4] = 0xB4;
            overlay57Call3E5CReloc(1);
        } else if (overlay57Call3E6CReloc() == 6) {
            O57_S32(0x118) = 21;
            O57_S32(0x160) = 0;
            O57_S32(0x164) = 0;
            O57_S32(0x184) = 0x4B;
            gO57ModeTimerB6BaseReloc[0x114 / 4] = 0xB4;
            overlay57Call3EB4Reloc(1);
        } else {
            Overlay57LookupResult *result;
            Overlay57SelectionChild *child;
            Overlay57Selection *selection;
            Overlay57MarkedEntry *entries;
            s32 choice;
            s32 count;

            O57_S32(0x118) = 15;
            gO57ModeSetup21C.x0C = 0x17C;
            gO57ModeSetup21C.y0E = 0xBE;
            choice = overlay57Call3EE0Reloc();
            gO57ModeChoice4F8 = choice;
            gOverlay57Byte3EF4Reloc = (u8)choice;
            overlay57Call3EFCReloc(0x4D);
            overlay57UpdateNode();
            result = overlay57Call3F0CReloc(0x4C);
            if (result != 0) {
                child = result->child08;
                if (child != 0) {
                    child->selector3A = (s8)gO57ModeChoice4F8;
                    child = result->child08;
                    selection = child->selections68[child->selector3A];
                    if (selection != 0) {
                        entries = selection->entries4C;
                        if (entries != 0) {
                            count = selection->countOwner00->count2C;
                            if (count--) {
                                do {
                                    if ((entries->flags04 & 0x00100000) != 0) {
                                        entries->value00 = 0x100;
                                    }
                                    entries++;
                                } while (count--);
                            }
                        }
                    }
                }
                overlay57ApplyTable();
            }
            overlay57Call3FA4Reloc(3);
        }
dispatch_done:

    if (savedEligible != 0) {
        overlay57Call3FB8Reloc();
    }
}
