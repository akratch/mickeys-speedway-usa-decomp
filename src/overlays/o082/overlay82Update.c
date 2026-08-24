#include "PR/ultratypes.h"


































typedef struct O82DisplayPair {
    s16 label;
    s16 value;
} O82DisplayPair;

typedef struct O82State {
      s8 selection;
      s8 changed;
      u8 active;
      u8 disabled;
      s32 values[6];
      O82DisplayPair display[6];
      u16 flags;
} O82State;

typedef struct O82Resource O82Resource;

typedef struct O82ResourceOwner {
      O82Resource *resource;
} O82ResourceOwner;

typedef struct O82Object {
      u8 pad00[0x28];
      f32 progress;
      u8 pad2C[0x38];
      O82State *state;
      O82ResourceOwner *resourceOwner;
} O82Object;

extern u32 overlay82InputButtonsReloc;
extern s16 overlay82InputXReloc;
extern s16 overlay82InputYReloc;


extern const u8 gO82DataBase[];





extern void overlay82PlayEventReloc(u16 eventId, void *nullableHandle);
extern void overlay82SetChannelReloc(O82Resource *resource, s32 channel, s32 value);

void overlay82Update(O82Object *object, f32 updateRate) {
    O82State *state;
    O82Resource *resource;
    const s16 (*targets)[6];
    const s16 *targetRow;
    const s16 *targetValues;
    s32 *currentValues;
    const s8 *channelMap;
    const s8 *channelValues;
    O82DisplayPair *displayCursor;
    s32 movementEvent;
    s32 edgeEvent;
    s32 shift;
    s32 index;
    s32 value;

    state = object->state;

    if (object->progress > 0.5f) {
        if (state->active == 0) {
            movementEvent = -1;
            edgeEvent = -1;

            if (overlay82InputXReloc < -30) {
                state->changed = 1;
                state->selection--;
                if (state->selection < 0) {
                    state->selection = 0;
                    edgeEvent = 14;
                } else if (state->selection == 3) {
                    state->selection = 4;
                    edgeEvent = 14;
                } else {
                    movementEvent = 0x26D;
                }
            } else if (overlay82InputXReloc >= 31) {
                state->selection++;
                state->changed = 1;
                if (state->selection == 10) {
                    state->selection = 9;
                    edgeEvent = 14;
                } else if (state->selection == 4) {
                    state->selection = 3;
                    edgeEvent = 14;
                } else {
                    movementEvent = 0x26D;
                }
            }

            if (overlay82InputYReloc >= 31) {
                if (state->selection >= 5 && state->selection < 9) {
                    state->selection -= 5;
                    state->changed = 1;
                    movementEvent = 0x26D;
                } else {
                    edgeEvent = 14;
                }
            } else if (overlay82InputYReloc < -30) {
                if (state->selection < 4) {
                    state->selection += 5;
                    state->changed = 1;
                    movementEvent = 0x26D;
                } else {
                    edgeEvent = 14;
                }
            }

            if (movementEvent != -1) {
                overlay82PlayEventReloc(movementEvent, ((void *)0) );
            }
            if (edgeEvent != -1) {
                overlay82PlayEventReloc(edgeEvent, ((void *)0) );
            }
            if ((overlay82InputButtonsReloc & 0x9000) != 0) {
                state->active = 1;
                overlay82PlayEventReloc(12, ((void *)0) );
            }
        } else if ((overlay82InputButtonsReloc & 0x4000) != 0 && state->disabled == 0) {
            state->active = 0;
            overlay82PlayEventReloc(13, ((void *)0) );
        }
    }

    targets = ((const s16 (*)[6]) (gO82DataBase + 0x00)) ;
    if (object->progress > 0.75f) {
        resource = object->resourceOwner->resource;
        targetRow = (const s16 *)targets + state->selection * 6;
        shift = state->changed != 0 ? 2 : 3;
        index = 0;
        currentValues = (s32 *)state;
        targetValues = targetRow;
        /* Zero-code web-priority read required by IDO 5.3. */
        if (currentValues) {}
        do {
            currentValues++;
            targetValues++;
            *currentValues +=
                (((s32)targetValues[-1] << 16) - *currentValues) >> shift;
            index++;
        } while (index < 6);

        channelMap = ((const s8 *) (gO82DataBase + 0x78)) ;
        index = 0;
        do {
            if (index == state->selection) {
                overlay82SetChannelReloc(resource, *channelMap, 0x100);
            } else {
                overlay82SetChannelReloc(resource, *channelMap, 0);
            }
            index++;
            channelMap++;
        } while (index != 10);

        channelValues = ((const s8 *) (gO82DataBase + 0x84)) ;
        if (state->active != 0) {
            overlay82SetChannelReloc(resource, 6,
                (*(const s8 *)((u32)channelValues + state->selection) + 10) << 8);
            overlay82SetChannelReloc(resource, 7,
                (*(const s8 *)((u32)channelValues + state->selection) + 10) << 8);
        } else {
            overlay82SetChannelReloc(resource, 6,
                *(const s8 *)((u32)channelValues + state->selection) << 8);
            overlay82SetChannelReloc(resource, 7,
                *(const s8 *)((u32)channelValues + state->selection) << 8);
        }
    }

    displayCursor = state->display;
    currentValues = state->values;
    for (index = 0; index < 6; index++) {
        displayCursor->label = 14 + index * 3; displayCursor->value = state->values[index] >> 16;
        displayCursor++;
        currentValues++;
    }
    displayCursor->label = 0x2000;
}
