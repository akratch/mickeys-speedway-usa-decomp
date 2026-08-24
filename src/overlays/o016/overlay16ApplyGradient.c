#include "PR/ultratypes.h"

typedef struct Overlay16Vertex {
    s16 x; s16 y; s16 z;
    u8 red; u8 green; u8 blue; u8 alpha;
} Overlay16Vertex;
typedef struct Overlay16ColorSource { u8 *colors; s16 *blocks; } Overlay16ColorSource;
typedef struct Overlay16Batch {
    Overlay16Vertex *vertices;
    u8 pad04[0x1C];
    s16 vertexCount;
    u8 pad22[0xC];
    u8 dirty;
    u8 pad2F;
    Overlay16ColorSource *source;
    u8 pad34[0xC];
} Overlay16Batch;
typedef struct Overlay16Context {
    u8 pad00[4];
    Overlay16Batch *batches;
    u8 pad08[0x12];
    s16 batchCount;
} Overlay16Context;

extern u8 *gOverlay16Buffer;
extern s32 gOverlay16Phase;
extern s32 gOverlay16Mode;

/* DKR v77/v80 and JFG contain no exact donor for this gradient pass. */
void overlay16ApplyGradient(s32 *active, Overlay16Context *context,
                            s32 phaseStep) {
    Overlay16Batch *batch;
    Overlay16ColorSource *source;
    Overlay16Vertex *vertex;
    u8 *input;
    u8 *gradient;
    s32 batchIndex;
    s32 vertexIndex;
    s32 gradientIndex;
    s32 blocks;
    s32 phase;

    gradient = gOverlay16Buffer;
    if (gradient == NULL) {
        return;
    }
    phase = (gOverlay16Phase + phaseStep) & 0xFF;
    gOverlay16Phase = phase;
    batch = context->batches;
    batchIndex = context->batchCount;
    while (batchIndex--) {
        if (*active++) {
            source = batch->source;
            if (source) {
                s16 *block;
                block = source->blocks;
                blocks = (batch->vertexCount + 0xF) >> 4;
                while (blocks--) {
                    *block++ = 0;
                }
                input = source->colors;
                vertex = batch->vertices;
                vertexIndex = batch->vertexCount;
                if (gOverlay16Mode == 1) {
                    while (vertexIndex--) {
                        u8 *gradientColor;
                        gradientIndex =
                            (vertex->z + vertex->x + vertex->y + phase) & 0xFF;
                        gradientColor = gradient + gradientIndex;
                        gradientColor += gradientIndex;
                        gradientColor += gradientIndex;
                        vertex->red = (input[0] * gradientColor[0]) >> 8;
                        vertex->green = (input[1] * gradientColor[1]) >> 8;
                        vertex->blue = (input[2] * gradientColor[2]) >> 8;
                        input += 3;
                        vertex++;
                    }
                } else {
                    while (vertexIndex--) {
                        u8 *gradientColor;
                        u8 inputRed;
                        u8 inputGreen;
                        u8 inputBlue;
                        gradientIndex =
                            (vertex->z + vertex->x + vertex->y + phase) & 0xFF;
                        gradientColor = gradient + gradientIndex;
                        gradientColor += gradientIndex;
                        gradientColor += gradientIndex;
                        inputRed = input[0];
                        inputGreen = input[1];
                        inputBlue = input[2];
                        vertex->red = gradientColor[0] < inputRed ? inputRed : gradientColor[0];
                        vertex->green = gradientColor[1] < inputGreen ? inputGreen : gradientColor[1];
                        vertex->blue = gradientColor[2] < inputBlue ? inputBlue : gradientColor[2];
                        input += 3;
                        vertex++;
                    }
                }
            }
            batch->dirty = 0;
        }
        batch++;
    }
}
