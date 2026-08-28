#include "PR/ultratypes.h"

typedef struct Overlay84Node {
    s16 pad0;
    s16 tilt;
    u8 pad4[8];
    f32 x;
    f32 y;
    f32 z;
} Overlay84Node;

typedef struct Overlay84State {
    u8 pad0;
    s8 current;
    s8 last;
    u8 pad3[0xD];
    s16 tilt;
    u8 pad12[4];
    s16 angle;
    u32 flags;
    u8 pad1C[8];
    f32 height;
    u8 pad28[0x1C];
    Overlay84Node *volatile nodes[32];
    u8 padC4[4];
    u32 disabled;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84ObjectRead;
extern Overlay84Object *gOverlay84ObjectWrite;
extern s32 overlay84Atan2(f32 x, f32 z);

/* NON_MATCHING plateau (retested 2026-08-25): the nearest skeleton score is
 * 0.070 and all 119 flag combinations miss. Ten focused pad size/order,
 * declaration-order, and split node-lifetime variants leave the natural
 * exact-size result best: two of 82 words differ, first at +0xEC, because IDO
 * spills the call-live node at sp+0x24 instead of retail's sp+0x1C. A bounded
 * two-worker permuter batch improved cost only by making the current wrap
 * assignment unconditional, so that semantically invalid form was rejected. */
/* Object-level reproof: instruction-words-identical, 0 differing words, first
 * mismatch none; the 82-instruction, frame -48 shape is exact and permuter-ready.
 * Overlay relocation/link proof remains deferred, so retain NON_MATCHING. */
void overlay84AdvanceCurrent(s32 direction)
{
  Overlay84Object *object;
  Overlay84State *state;
  s32 found;
  s32 pad;
  Overlay84Node *node;
  object = gOverlay84ObjectRead;
  found = 0;
  if (&pad)
  {
    ;
  }
  if (object != 0)
  {
    state = object->state;
    if (state->last != (-1))
    {
      do
      {
        if (direction == 1)
        {
          state->current--;
          if (state->current < 0)
          {
            state->current = state->last;
          }
        }
        else
        {
          state->current++;
          if (state->last < state->current)
          {
            state->current = 0;
          }
        }
        if ((state->nodes[state->current] != 0) && ((state->disabled & (1 << state->current)) == 0))
        {
          found = 1;
        }
      }
      while (found == 0);
      object = gOverlay84ObjectWrite;
      node = state->nodes[*((volatile s8 *) (&state->current))];
      state->angle = overlay84Atan2(node->x - object->x, node->z - object->z);
      state->tilt = -node->tilt;
      state->height = node->y;
      state->flags &= ~1;
      state->flags |= direction;
    }
  }
}
