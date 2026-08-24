typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

typedef struct O8Node {
    void *resource;
    u8 pad04[4];
    s16 active;
    s16 index;
    void *items[1];
} O8Node;

typedef struct O8ChildFlag { u8 pad00[0x1e]; s8 gate; } O8ChildFlag;
typedef struct O8OwnerPeer { u8 pad00[0x63]; u8 gate63; } O8OwnerPeer;

typedef struct O8Owner {
    u8 pad00[0xc];
    s32 valueC, value10, value14;
    u8 pad18[0x10];
    f32 position28;
    u8 pad2c[0xf];
    s8 mode3B;
    u8 pad3c[4];
    u8 *children40;
    u8 pad44[4];
    O8OwnerPeer *peer48;
    u8 pad4c[4];
    void *value50;
    u8 pad54[0x14];
    O8Node **node68;
    u8 pad6c[0x14];
    s32 flags80;
    u8 pad84[0xf];
    u8 childIndex93;
} O8Owner;

typedef struct O8State {
    u8 pad00[2];
    u8 condition2;
    u8 pad03;
    f32 lateral4;
    u8 pad08[0x68];
    void *value70;
    f32 vector74, vector78, vector7C;
    f32 value80, value84;
    u8 pad88[0x30];
    void *resourceB8;
    u8 padBC[8];
    void *resourceC4;
    u8 padC8[0xc];
    s32 conditionD4;
    u8 padD8[0x2e];
    s16 angle106;
    u8 pad108[0x2c];
    void *resource134;
    void *resource138;
    u8 pad13c[0x36];
    s8 condition172;
    u8 pad173[0xe];
    u8 active181;
    u8 pad182[0x1a0];
    u8 selector322;
    u8 selector323;
    u8 pad324[0x26];
    u8 mode34A;
    u8 pad34B[0x68];
    u8 timer3B3;
    u8 pad3B4[0x88];
    s16 angle43C;
} O8State;

#define O8_F32(address) (*(f32 *)(address))
#define O8_S32(address) (*(s32 *)(address))
#define O8_S32_ARRAY(address) ((s32 *)(address))
#define O8_PTR(address) (*(void **)(address))

extern void func_overlay_008_F0000000_185DD58(O8Node *, void *, O8Owner *);
extern void ext_o0_19668(O8Owner *, O8Node *, void *, void *);
extern void ext_o8_3368(O8Owner *, O8State *, void *, O8Node *, s32);
extern f32 gO8FloatCC, gO8FloatD0, gO8FloatD4, gO8FloatD8;
extern f32 gO8FloatDC, gO8FloatE0;
extern void *gO8Pointer14, *gO8Pointer18;
extern s32 gO8Table360[], gO8Table3A0[];
extern s32 gO8Value364, gO8Value3A4, gO8Value370, gO8Value3B0;
extern void ext_o0_1eed0_target(O8State *, s32, u32);
extern f32 ext_o0_2a46c(s16);
extern f32 ext_o0_2a470(s16);
extern void func_overlay_008_F0002640_1860398(O8Owner *, O8State *, s32,
                                               s32, s32, f32, f32, f32, s32);
extern void ext_o8_3278(O8Owner *, O8State *, s32);
extern void ext_o8_2ec0(O8Owner *, O8State *, s32, s32, s32);
extern void ext_o0_1d510(O8Owner *, O8State *, s32);
extern void ext_o0_2d98(void *);
extern void ext_o0_2b90(s32, s32, s32, s32, s32, void **);
extern void ext_o7_ccc(O8Owner *, s32);
extern void ext_o8_3018(O8Owner *, O8State *, void *, s32);
extern void ext_o0_3e990(f32);
extern void ext_o0_3e99c(O8Owner *, s32);
extern void ext_o17_668(void *, void *);
extern void ext_o0_2d70(void *, s32, s32, s32);

void func_overlay_008_F0000894_185E5EC(O8Owner *owner, O8State *state,
                                       s32 updateRate) {
    void *savedResource;
    O8Node *node;
    f32 update = (f32)updateRate;
    f32 sine;
    f32 cosine;
    f32 y;
    f32 x;
    O8Node *savedNode;
    f32 sideA;
    f32 sideB;
    f32 z;
    s32 effect;
    s32 angleA;
    s32 angleB;
    s32 flags;

    node = *owner->node68;
    savedResource = node->resource;
    if (((s8 *)owner->children40)[owner->childIndex93 + 0x1e] == 0 &&
        (node != 0) && (node->active != 0)) {
        savedNode = node;
        func_overlay_008_F0000000_185DD58(node, savedResource, owner);
        ext_o0_19668(owner, savedNode, owner->value50,
                     savedNode->items[savedNode->index]);
        ext_o8_3368(owner, state, savedResource, node, node->active);
        savedNode->active = 0;
    }

    effect = -1;
    if ((state->active181 != 0) && (owner->position28 <= gO8FloatCC)) {
        if (owner->mode3B == 0x14) effect = 0;
        else if (owner->mode3B == 0x13) effect = 1;
        else if (owner->mode3B == 0x15) effect = 2;
        if (effect != -1) {
            f32 scale = state->value84 * update;
            sideA = state->vector74 * scale;
            sideB = state->vector78 * scale;
            z = state->vector7C * scale;
            angleA = 0x1200;
            angleB = 0x1c00;
        }
        ext_o0_1eed0_target(state, 0x4b, 0x3e19999a);
    }

    if ((state->lateral4 < -3.25f) && (effect == -1)) {
        if ((state->mode34A & 3) == 3) {
            effect = 1;
            y = 0.0f;
            x = 4.0f - state->lateral4 * gO8FloatD0;
        } else if (state->mode34A & 5) {
            effect = 0;
            x = 0.0f;
            y = 4.0f - state->lateral4 * gO8FloatD4;
        } else if (state->mode34A & 0xa) {
            effect = 2;
            x = 0.0f;
            y = state->lateral4 * gO8FloatD8 + -4.0f;
        }
        if (effect != -1) {
            sine = ext_o0_2a46c(state->angle43C);
            cosine = ext_o0_2a470(state->angle43C);
            angleA = 0x2a00;
            angleB = 0x3600;
            sideA = y * sine + x * cosine;
            sideB = 0.0f;
            z = x * sine - y * cosine;
            ext_o0_1eed0_target(state, 0x28, 0x3e19999a);
        }
    }
    if (effect != -1) {
        func_overlay_008_F0002640_1860398(owner, state, effect, angleA, angleB,
                                          sideA, sideB, z, 2);
        state->timer3B3 = 0x64;
    }

    ext_o8_3278(owner, state, updateRate);
    ext_o8_2ec0(owner, state, 0, 0, updateRate);
    ext_o0_1d510(owner, state, updateRate);
    if ((state->active181 != 0) &&
        (((state->mode34A != 0) && (state->value84 == state->value80)) ||
         (owner->peer48->gate63 != 0))) {
        if (state->resourceB8 != 0) ext_o0_2d98(state->resourceB8);
        ext_o0_2b90(7, owner->valueC, owner->value10, owner->value14, 4,
                    &state->resourceB8);
        ext_o7_ccc(owner, 0x12);
    }
    ext_o8_3018(owner, state, state->value70, updateRate);

    if ((state->condition172 != 0) && (state->lateral4 < -2.0f)) {
        flags = owner->flags80 & ~0x33;
        owner->flags80 = flags;
        flags |= gO8Value370;
        owner->flags80 = flags;
        owner->flags80 = flags | gO8Value3B0;
    } else if (((state->condition2 != 0) || (state->conditionD4 != 0)) &&
               ((state->lateral4 < gO8FloatDC) ||
                (state->lateral4 > gO8FloatE0))) {
        flags = owner->flags80 & ~0x33;
        owner->flags80 = flags;
        flags |= gO8Value364;
        owner->flags80 = flags;
        owner->flags80 = flags | gO8Value3A4;
    } else if (state->lateral4 < -5.0f) {
        flags = owner->flags80 | gO8Table360[state->selector322 & 0xf];
        owner->flags80 = flags;
        owner->flags80 = flags | gO8Table3A0[state->selector323 & 0xf];
    }

    flags = owner->flags80;
    if (flags & 0x10) {
        owner->flags80 = flags & ~1;
        flags = owner->flags80;
    }
    if (flags & 0x20) owner->flags80 = flags & ~2;
    if (state->angle106 >= 0x1b)
        ext_o0_3e990((f32)((0x5a - (s32)state->angle106) << 2));
    ext_o0_3e99c(owner, updateRate);
    if (state->resource134 != 0) ext_o17_668(state->resource134, gO8Pointer14);
    if (state->resource138 != 0) ext_o17_668(state->resource138, gO8Pointer18);
    if (state->resourceC4 != 0)
        ext_o0_2d70(state->resourceC4, owner->valueC, owner->value10,
                    owner->value14);
}
