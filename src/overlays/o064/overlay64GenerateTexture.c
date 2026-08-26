typedef signed int s32;
typedef unsigned char u8;
typedef unsigned short u16;

typedef struct O64Image {
    u8 pad00[6];
    u16 width;
    u16 height;
    u8 pad0A[0x16];
    u16 pixels[1];
} O64Image;

extern volatile u8 gO64BufferSelect;
extern s32 gO64Initialized;
extern u8 *gO64BuffersA[];
extern u8 *gO64BuffersB[];
extern s32 o64RandomRange(s32 minimum, s32 maximum);

/* Workbench p4: structure-mismatch; 420/420 instructions, 401 positional words, first +0x0; frame 0x78 vs target 0x70.
 * Levers: restored the even-row temporaries and moved the buffer-flag read ahead of dimension/table selection; this is the best source candidate.
 * Remains: raw overlay relocation identities, the 8-byte frame excess, and the residual pointer/local schedule. */
#ifdef NON_MATCHING
void func_overlay_064_F0000000_18C3B28(s32 index, O64Image *image,
                                        u8 *unused)
{
    register s32 width;
    register s32 height;
    u8 *selected;
    register u8 *source;
    register u8 *dest;
    register u8 *write;
    register s32 innerWidth;
    register s32 innerHeight;
    register s32 y;
    register s32 x;
    register s32 distance;
    register s32 maximum;
    register s32 value;
    register u8 *below;
    register u16 *pixels;
    u8 newSelect;

    (void)unused;

    newSelect = gO64BufferSelect;
    width = image->width;
    height = image->height;
    if (newSelect != 0) {
        selected = gO64BuffersA[index];
    } else {
        selected = gO64BuffersB[index];
    }

    innerWidth = width - 6;
    innerHeight = height - 6;
    write = selected + innerHeight * width + 3;
    if (gO64Initialized == 0) {
        x = innerWidth - 1;
        if (innerWidth != 0) {
            y = width >> 1;
            do {
                distance = y - x - 3;
                if (distance <= 0) {
                    distance = -distance;
                }
                *write++ = o64RandomRange(0, 0xFF - distance * 0x10);
            } while (x--);
        }
        gO64Initialized = 1;
    } else {
        x = innerWidth - 1;
        if (innerWidth != 0) {
            y = width >> 1;
            do {
                distance = y - x - 3;
                if (distance <= 0) {
                    distance = -distance;
                }
                maximum = 0xFF - distance * 0x10;
                value = o64RandomRange(-0x60, 0x60) + *write;
                if (value > maximum) {
                    value = maximum;
                }
                if (value < 0) {
                    value = 0;
                }
                *write++ = value;
            } while (x--);
        }
    }

    newSelect = gO64BufferSelect ^ 1;
    gO64BufferSelect = newSelect;
    if (newSelect != 0) {
        dest = gO64BuffersA[index];
    } else {
        dest = gO64BuffersB[index];
    }

    source = selected;
    write = dest;
    y = height - 7;
    if (innerHeight != 0) {
        do {
            *write++ = 0;
            source++;
            x = width - 3;
            if (width - 2 != 0) {
                below = source + width;
                do {
                    value = below[0] * 4 + source[0] * 2 + below[-1] + below[1];
                    source++;
                    below++;
                    value >>= 3;
                    if (value < 0xAA) {
                        value--;
                    }
                    if (value < 0) {
                        value = 0;
                    }
                    *write++ = value;
                } while (x--);
            }
            *write++ = 0;
            source++;
        } while (y--);
    }

    write += width;
    source += width;
    y = 4;
    do {
        *write++ = 0;
        source++;
        x = width - 3;
        if (width - 2 != 0) {
            below = source - width;
            do {
                value = below[0] + below[-1] + below[1];
                source++;
                below++;
                value >>= 2;
                if (value < 0xAA) {
                    value--;
                }
                if (value < 0) {
                    value = 0;
                }
                *write++ = value;
            } while (x--);
        }
        *write++ = 0;
        source++;
    } while (y--);

    write = dest + innerHeight * width;
    source = write - width;
    x = width - 1;
    if (width != 0) {
        do {
            *write++ = *source++;
        } while (x--);
    }

    source = dest;
    y = height - 1;
    if (height - 7 != -7) {
        pixels = image->pixels;
        do {
            if (y & 1) {
                x = width - 1;
                if (width != 0) {
                    do {
                        value = *source++;
                        distance = value;
                        maximum = value + 0x34;
                        innerHeight = value >> 4;
                        if (innerHeight != 0) {
                            distance = value * innerHeight;
                        }
                        if (maximum >= 0x100) {
                            maximum = 0xFF;
                        }
                        if (distance >= 0x100) {
                            distance = 0xFF;
                        }
                        *pixels++ = (maximum << 8) | distance;
                    } while (x--);
                }
            } else {
                x = 0;
                if (width > 0) {
                    do {
                        innerWidth = (x & 2) ? -2 : 2;
                        value = *source++;
                        distance = value;
                        maximum = value + 0x34;
                        innerHeight = value >> 4;
                        if (innerHeight != 0) {
                            distance = value * innerHeight;
                        }
                        if (maximum >= 0x100) {
                            maximum = 0xFF;
                        }
                        if (distance >= 0x100) {
                            distance = 0xFF;
                        }
                        *(u16 *)((u8 *)pixels + innerWidth * 2 + x * 2) =
                            (maximum << 8) | distance;
                        x++;
                    } while (x != width);
                }
                pixels = (u16 *)((u8 *)pixels + width * 2);
            }
        } while (y--);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o064/overlay64GenerateTexture/func_overlay_064_F0000000_18C3B28.s")
#endif
