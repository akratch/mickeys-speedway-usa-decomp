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

void func_overlay_064_F0000000_18C3B28(s32 index, O64Image *image,
                                        u8 *unused)
{
    s32 width = image->width;
    s32 height = image->height;
    s32 bufferOffset = index * 4;
    u8 * volatile selected;
    u8 *source;
    u8 *dest;
    u8 *write;
    s32 innerWidth = width - 6;
    s32 innerHeight = height - 6;
    s32 y;
    s32 x;
    u8 newSelect;

    (void)unused;

    if (gO64BufferSelect != 0) {
        selected = gO64BuffersA[index];
    } else {
        selected = gO64BuffersB[index];
    }

    write = selected + innerHeight * width + 3;
    if (gO64Initialized == 0) {
        x = innerWidth - 1;
        if (innerWidth != 0) {
            s32 middle = width >> 1;
            do {
                s32 distance = middle - x - 3;
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
            s32 middle = width >> 1;
            do {
                s32 distance = middle - x - 3;
                s32 maximum;
                s32 value;
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
                u8 *below = source + width;
                do {
                    s32 value = below[0] * 4 + source[0] * 2 + below[-1] + below[1];
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
            u8 *above = source - width;
            u8 *left = above - 1;
            u8 *right = above + 1;
            do {
                s32 value = above[0] + left[0] + right[0];
                source++;
                above++;
                left++;
                right++;
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
        u16 *pixels = image->pixels;
        do {
            if (y & 1) {
                x = width - 1;
                if (width != 0) {
                    do {
                        s32 value = *source++;
                        s32 low = value;
                        s32 high = value + 0x34;
                        s32 scale = value >> 4;
                        if (scale != 0) {
                            low = value * scale;
                        }
                        if (high >= 0x100) {
                            high = 0xFF;
                        }
                        if (low >= 0x100) {
                            low = 0xFF;
                        }
                        *pixels++ = (high << 8) | low;
                    } while (x--);
                }
            } else {
                s32 rowBytes = width * 2;
                x = 0;
                if (width > 0) {
                    do {
                        s32 displacement = (x & 2) ? -2 : 2;
                        s32 value = *source++;
                        s32 low = value;
                        s32 high = value + 0x34;
                        s32 scale = value >> 4;
                        if (scale != 0) {
                            low = value * scale;
                        }
                        if (high >= 0x100) {
                            high = 0xFF;
                        }
                        if (low >= 0x100) {
                            low = 0xFF;
                        }
                        *(u16 *)((u8 *)pixels + displacement * 2 + x * 2) =
                            (high << 8) | low;
                        x++;
                    } while (x != width);
                }
                pixels = (u16 *)((u8 *)pixels + rowBytes);
            }
        } while (y--);
    }
}
