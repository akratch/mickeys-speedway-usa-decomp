#ifndef OVERLAY_008_H
#define OVERLAY_008_H

#include "PR/ultratypes.h"

typedef struct Overlay8IndexedObject {
    u8 pad0;
    s8 index;
} Overlay8IndexedObject;

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
#define gOverlay8MotionOutsideValueReloc D_28C

typedef struct Overlay8MotionRecord {
    u8 pad000[0xC4];
    void *resource;
    u8 pad0C8[0x38];
    s16 direction;
    s16 activeDirection;
    u8 pad104[4];
    s16 fallbackSign;
    u8 pad10A[0x5E];
    s16 gate168;
    s16 gate16A;
} Overlay8MotionRecord;

typedef struct Overlay8ActivationState {
    s8 type;
    u8 pad001[0xC3];
    void *resource;
    u8 pad0C8[0x3A];
    s16 activeDirection;
    u8 pad104[0x54];
    s16 gate158;
    u8 pad15A[0x29];
    u8 active183;
    u8 pad184;
    u8 active185;
    u8 pad186;
    u8 timer187;
    u8 pad188[0x20];
    u16 flags1A8;
} Overlay8ActivationState;

typedef struct Overlay8ActivationOwner {
    u8 pad00[0x0C];
    s32 valueC;
    s32 value10;
    s32 value14;
    u8 pad18[0x4C];
    Overlay8ActivationState *state;
} Overlay8ActivationOwner;

typedef struct O8PhaseState {
    u8 pad000[0x16A];
    s16 timer;
    u8 pad16C[0x18];
    u8 forceEffect;
    u8 phase;
    u8 effect;
    u8 countdown;
    f32 weight;
    u8 pad18C[0x290];
    u32 flags;
} O8PhaseState;

typedef struct O8P2640Anchor {
    s16 helperInput0;
    u8 pad002[0x0A];
    f32 coordC;
    f32 coord10;
    f32 coord14;
} O8P2640Anchor;

typedef struct O8P2640Config {
    u8 pad00;
    s8 tuningIndex1;
} O8P2640Config;

typedef struct O8P2640Tuning {
    f32 extent0;
    f32 offset4;
    f32 spreadScale8;
} O8P2640Tuning;

typedef struct O8P2640Record {
    s16 value0;
    s16 value2;
    f32 magnitude4;
    f32 coord8;
    f32 coordC;
    f32 coord10;
    f32 phase14;
    s16 size18;
    u8 kind1A;
    u8 pad1B;
    u32 packed1C;
    u32 packed20;
    u32 packed24;
    u32 packed28;
    u32 packed2C;
    u32 packed30;
} O8P2640Record;

typedef struct O8P291CLink {
    u8 pad00[0x62];
    u8 gate62;
} O8P291CLink;

typedef struct O8P291CMotion {
    s16 heading0;
    u8 pad02[0x0A];
    f32 positionC;
    f32 position10;
    f32 position14;
    u8 pad18[4];
    f32 velocity1C;
    f32 velocity20;
    f32 velocity24;
    u8 pad28[6];
    s16 state2E;
    u8 pad30[0x18];
    O8P291CLink *link48;
} O8P291CMotion;

typedef struct O8P291CState {
    u8 pad00[4];
    f32 control4;
    f32 control8;
    f32 blendC;
    u8 pad10[0x28];
    f32 origin38;
    f32 origin3C;
    f32 origin40;
    u8 pad44[0x30];
    f32 axis74;
    f32 axis78;
    f32 axis7C;
    f32 speed80;
    f32 speed84;
    f32 accel88;
    u8 pad8C[8];
    f32 delta94;
    f32 delta98;
    f32 delta9C;
    u8 padA0[0x50];
    s16 angleF0;
    u8 padF2[0x0A];
    s16 angleFC;
    s16 angleFE;
    u8 pad100[4];
    s16 angle104;
    u8 pad106[0x64];
    s16 mode16A;
    u8 pad16C[4];
    u8 reset170;
    u8 pad171[0x10];
    u8 active181;
    u8 pad182[3];
    u8 suppress185;
    u8 pad186[0x296];
    u32 flags41C;
    u8 pad420[0x18];
    s32 mode438;
} O8P291CState;

typedef struct O8P291CBlendView {
    u8 pad00[0x0C];
    f32 blendC;
} O8P291CBlendView;

typedef struct Overlay8UpdateFlag {
    u8 flags;
} Overlay8UpdateFlag;

typedef struct Overlay8UpdateChild {
    u8 pad00[0x75];
    u8 near;
    u8 pad76[0x0A];
    f32 target80;
    Overlay8UpdateFlag *flag;
} Overlay8UpdateChild;

typedef struct Overlay8UpdateOwner {
    u8 pad00[0x10];
    f32 position10;
    u8 pad14[0x0C];
    f32 velocity20;
    u8 pad24[0x30];
    Overlay8UpdateChild *child;
} Overlay8UpdateOwner;

typedef struct Overlay8UpdateInput {
    u8 pad00[2];
    u8 active;
    u8 pad03;
    f32 lateral;
    u8 pad08[0x60];
    f32 position68;
    f32 delta6C;
} Overlay8UpdateInput;

typedef struct Overlay8ChannelState {
    u8 pad000[4];
    f32 position;
    u8 pad008[0x114];
    f32 values[4];
    u8 modes[4];
    u8 phases[4];
    u8 pad134[0x188];
    s32 selectorMode;
    u8 pad2C0[0x60];
    u8 selectors[4];
} Overlay8ChannelState;

typedef struct Overlay8ColorSource {
    u8 pad00[0x38];
    u8 red;
    u8 green;
    u8 blue;
} Overlay8ColorSource;

typedef struct Overlay8ColorOwner {
    u8 pad00[0x64];
    Overlay8ColorSource *colors;
} Overlay8ColorOwner;

typedef struct Overlay8ColorState {
    u8 pad000[0xD0];
    Overlay8ColorOwner *owner;
    u8 pad0D4[0x96];
    s16 timer16A;
    u8 pad16C[0x18];
    u8 alternate184;
    u8 pad185;
    u8 flags186;
    u8 pad187[0x1CD];
    void *target354;
    u8 pad358[8];
    void *target360;
} Overlay8ColorState;

typedef struct Overlay8ScaleRecord {
    u8 pad00[0x0E];
    u16 scale;
} Overlay8ScaleRecord;

typedef struct Overlay8ScaleSlot {
    Overlay8ScaleRecord *record;
    u32 pad04;
} Overlay8ScaleSlot;

typedef struct Overlay8ScaleContext {
    u8 pad00[0x18];
    Overlay8ScaleSlot *slots;
    u8 pad1C[0x10];
    u8 count;
} Overlay8ScaleContext;

typedef struct Overlay8ScalePair {
    s16 first;
    s16 second;
    u32 selector;
} Overlay8ScalePair;

typedef struct Overlay8ScaleOutput {
    u8 pad00[0x0A];
    s16 outputIndex;
    u8 pad0C[0x40];
    Overlay8ScalePair *pairs;
    s16 *outputs[];
} Overlay8ScaleOutput;

typedef struct Overlay8ScaleState {
    u8 pad000[4];
    f32 position;
    u8 pad008[0x414];
    u32 flags41C;
    u8 pad420[0x0C];
    s32 value42C;
} Overlay8ScaleState;

typedef struct Overlay8MotionRow {
    u8 firstSelector;
    u8 secondSelector;
    u8 pad002[2];
    s16 firstScale;
    s16 secondScale;
} Overlay8MotionRow;

typedef struct Overlay8MotionAnchor {
    s16 helperInput;
    u8 pad002[0x0A];
    f32 x;
    u8 pad010[4];
    f32 y;
} Overlay8MotionAnchor;

typedef struct Overlay8MotionState {
    u8 pad000;
    s8 rowIndex;
    u8 pad002[0x191];
    u8 outsideLatch;
    f32 outsideValue;
    u8 pad198[0x24C];
    Overlay8MotionAnchor *target;
    s16 primary;
    s16 secondary;
} Overlay8MotionState;

typedef struct O8P4CF0Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O8P4CF0Vec3f;

typedef struct O8P4CF0Normal {
    volatile f32 x;
    f32 y;
    f32 z;
} O8P4CF0Normal;

typedef struct O8P4CF0Actor {
    s16 angle000;
    s16 angle002;
    s16 angle004;
    u8 pad006[6];
    f32 x00C;
    f32 y010;
    f32 z014;
    u8 pad018[8];
    f32 vertical020;
} O8P4CF0Actor;

typedef struct O8P4CF0State {
    u8 pad000[4];
    f32 motion004;
    u8 pad008[0x16A];
    s8 timer172;
    u8 activated173;
    f32 blend174;
    f32 height178;
    f32 derived17C;
} O8P4CF0State;

typedef struct O8P4CF0Bounds {
    s16 minX000;
    s16 unused002;
    s16 minZ004;
    s16 extentX006;
    s16 extentZ008;
} O8P4CF0Bounds;

typedef struct O8P4CF0SceneItem {
    u8 pad000[0x44];
    s16 category044;
    u8 pad046[0x3E];
    O8P4CF0Bounds *bounds084;
    s32 callbackGate088;
} O8P4CF0SceneItem;

extern u8 gOverlay8IndexMode;

extern void *gOverlay8Primary[];

extern void *gOverlay8Secondary[];

extern void o8Call0894Reloc(O8Node *, void *, O8Owner *);

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

extern void o8Call0894EmitReloc(O8Owner *, O8State *, s32,
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

extern void o8StartMotionResourceReloc(void *resource);

extern s32 gOverlay8ActivationGateTimerReloc;

extern void overlay8ReleaseResourceReloc(void *resource);

extern void overlay8CreateResourceReloc(s32 kind, s32 valueC, s32 value10,
                                        s32 value14, s32 count,
                                        void **resourceOut);

extern void overlay8FinalizeActivationReloc(Overlay8ActivationOwner *owner,
                                            s32 code);

extern u8 gO8RolloverControlReloc;

extern u8 gO8Phase2ScaleControlReloc;

extern u8 gO8Phase3ScaleControlReloc;

extern const f32 gO8Phase2TargetReloc;

extern const f32 gO8Phase2ScaleReloc;

extern const f32 gO8Phase3DecayReloc;

extern const f32 gO8RetireThresholdReloc;

extern const f32 gO8Phase3ScaleReloc;

extern f32 o8RolloverSampleReloc(void);

extern void o8Phase1EmitReloc(O8PhaseState *state, s32 kind, f32 scale);

extern const O8P2640Tuning D_2110[];

extern const f32 O8P2640_data_198;

extern const f32 O8P2640_data_19C;

extern f32 O8P2640_call_26AC(f32 squaredDistance);

extern s32 O8P2640_call_26F0(f32 negX, f32 negZ);

extern f32 O8P2640_call_26FC(s32 anchorHalfword);

extern f32 O8P2640_call_2708(s32 anchorHalfword);

extern s32 O8P2640_call_27BC(s32 low, s32 high);

extern s32 O8P2640_call_27CC(s32 low, s32 high);

extern s32 O8P2640_call_27DC(s32 low, s32 high);

extern void O8P2640_call_28C0(O8P2640Record *record);

extern f32 O8P291C_data_1A0;

extern f32 O8P291C_data_1A4;

extern f32 O8P291C_data_1A8;

extern f32 O8P291C_data_1AC;

extern f32 O8P291C_gravity;

extern f32 D_10;

extern f32 o8Approach291CReloc(f32 value, s32 updateRate);

extern f32 O8P291C_call_sin(s16 angle);

extern f32 O8P291C_call_cos(s16 angle);

extern void o8Surface291CReloc(
    O8P291CMotion *motion, O8P291CState *state, s32 updateRate);

extern s32 O8P291C_call_037C(O8P291CMotion *motion, O8P291CState *state,
                             f32 update);

extern s32 O8P291C_call_039C(O8P291CMotion *motion, f32 x, f32 y, f32 z);

extern const f32 gOverlay8UpdateLowerReloc;

extern const f32 gOverlay8UpdateUpperReloc;

extern const f32 gOverlay8UpdateDecayReloc;

extern void overlay8FinishUpdateReloc(Overlay8UpdateOwner *owner,
                                      s32 updateRate);

extern const f32 gOverlay8PhaseScales[];

extern const f32 gOverlay8SelectorScales[];

extern f32 overlay8SampleChannel(f32 argument, void *sampleState);

extern void overlay8EmitChannel(Overlay8ChannelState *state, s32 kind,
                                f32 scale);

extern void o8ApplyColorsReloc(void *target, s32 alpha0,
                                               s32 alpha1, s32 alpha2,
                                               s32 red, s32 green, s32 blue);

extern const f32 gOverlay8ScaleLowerReloc;

extern const f32 gOverlay8ScaleUpperReloc;

extern s16 *gOverlay8Buffer;

extern s16 gOverlay8Value;

extern const f32 D_28C;

extern const Overlay8MotionRow D_2230[];

extern s32 overlay8MeasureDirectionReloc(f32 deltaX, f32 deltaY);

extern s32 overlay8ConvertDirectionReloc(s32 anchorValue,
                                         s32 directionCode);

extern f32 overlay8ApproachMotionReloc(s32 difference,
                                       f32 doubledSecondary, f32 limit);

extern s32 O8P4CF0_call_4D14(s32 selector);

extern O8P4CF0SceneItem **O8P4CF0_call_4D24(s32 *start, s32 *end);

extern void O8P4CF0_call_4D54(s32 mode, O8P4CF0Actor *actor,
                              O8P4CF0Vec3f *input, O8P4CF0Vec3f *output);

extern void O8P4CF0_call_4E50(O8P4CF0SceneItem *item);

extern f32 O8P4CF0_call_4E64(O8P4CF0Bounds *bounds, f32 x, f32 z,
                             O8P4CF0Normal *normal);

extern f32 O8P4CF0_call_4E9C(s32 angle);

extern f32 O8P4CF0_call_4EAC(s32 angle);

extern s32 O8P4CF0_call_4EE8(f32 horizontal, f32 vertical);

extern s32 O8P4CF0_call_4EF8(f32 horizontal, f32 vertical);

extern f32 O8P4CF0_call_4F0C(f32 amount, s32 updateRate);

extern s32 O8P4CF0_call_4F34(s32 current, s32 target, f32 factor);

extern f32 O8P4CF0_call_4F48(f32 amount, s32 updateRate);

extern s32 O8P4CF0_call_4F68(s32 current, s32 target, f32 factor);

extern const f32 O8P4CF0_data_290;

extern const f32 O8P4CF0_data_294;

extern const f32 O8P4CF0_data_298;

extern const f32 O8P4CF0_data_29C;

extern const f32 O8P4CF0_data_2A0;

extern const f32 O8P4CF0_data_2A4;

extern const f32 O8P4CF0_data_2A8;

extern f32 O8P4CF0_data_4FD4;

#endif
