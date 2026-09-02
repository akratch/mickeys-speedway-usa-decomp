#include "PR/ultratypes.h"
#include "game/particles.h"

/* This TU uses -Wab,-r4300_mul; target func_8000A62C requires its three
 * R4300 multiply-hazard delay nops, and the flag is validated against the
 * existing exact object-system functions by the full-ROM verify. */

typedef struct AnimPathObject AnimPathObject;
typedef struct Gfx Gfx;
typedef struct Mtx Mtx;
typedef struct TrackVertex TrackVertex;

typedef struct TrackSkyObject {
    u8 pad00[6];
    s16 flags;
} TrackSkyObject;

typedef struct {
    u8 pad[0x58];
    s32 unk58;
} Objects06C40;

typedef struct {
    s16 unk0;
    u8 pad02[2];
    f32 unk4;
} Objects69C0Out;

typedef struct {
    u8 pad00[0x2C];
    f32 unk2C;
} Objects69C0Deep;

typedef struct {
    u8 pad00[0xE0];
    Objects69C0Deep *unkE0;
} Objects69C0Mid;

typedef struct {
    u8 pad00[0x40];
    Objects69C0Mid *unk40;
    u8 pad44[0x34];
    Objects69C0Out *unk78;
} Objects69C0In;

typedef struct {
    u8 pad00[0x1E];
    s8 unk1E[4];
    s8 unk22;
    u8 pad23[0xAD];
    f32 unkD0[4];
} Objects58C0Data;

typedef struct {
    u8 pad00[0x40];
    Objects58C0Data *unk40;
} Objects58C0Arg;

typedef struct {
    u8 pad00[0x44];
    s16 unk44;
    u8 pad46[0x22];
    s32 *unk68;
    u8 pad6C[0x1C];
    void *unk88;
} Objects08A20Arg;

typedef struct {
    u8 pad00[0x30];
    f32 unk30;
} Objects09F08Arg;

typedef struct {
    f32 unk0;
    u8 pad04[0x1A];
    s8 unk1E;
    u8 pad1F[3];
    s8 unk22;
    u8 pad23[0xB1];
    f32 unkD4;
} Objects09F74Data;

typedef struct {
    u8 pad00[8];
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x22];
    s8 unk3A;
    u8 pad3B[5];
    Objects09F74Data *unk40;
    s16 unk44;
    u8 pad46[0x1E];
    void *unk64;
    u8 pad68[0x2B];
    s8 unk93;
} Objects09F74Object;

typedef struct {
    u8 pad00[0x4E];
    u8 unk4E;
} Objects09F74Camera;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Objects0BB84Vec3;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
    u8 pad0C[4];
    f32 unk10;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
} Objects0BB84Plane;

typedef struct {
    u8 pad00[8];
    f32 x;
    f32 y;
    f32 z;
} Objects0BB84Output;

typedef struct {
    u8 pad00[0x10];
    f32 unk10;
} Objects0BB84Depth;

typedef struct {
    u8 pad00[0xE0];
    Objects0BB84Depth *unkE0;
} Objects0BB84Node;

typedef struct {
    u8 pad00[0x40];
    Objects0BB84Node *unk40;
    u8 pad44[0x34];
    Objects0BB84Output *unk78;
} Objects0BB84Object;

typedef struct {
    f32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    s32 unk14;
    u8 pad18[4];
    u16 unk1C;
    u16 unk1E;
    f32 unk20;
    f32 unk24;
    f32 unk28;
} Objects0B3CCConfig;

typedef struct {
    f32 unk0;
    u8 pad04[0x1A];
    u16 unk1E;
    u8 pad20[0x30];
    f32 unk50;
    u8 pad54[0x8C];
    Objects0B3CCConfig *unkE0;
} Objects0B3CCData;

typedef struct {
    s16 flags;
    s16 unk2;
    f32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    void *unk24;
} Objects0B3CCState;

typedef struct {
    u8 pad00[2];
    s16 unk2;
    s16 unk4;
    u8 pad06[6];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[4];
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    u8 pad28[0x18];
    Objects0B3CCData *unk40;
    u8 pad44[0x34];
    Objects0B3CCState *unk78;
    u8 pad7C[4];
    s32 unk80;
} Objects0B3CCObject;

extern void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority,
                          void **handle);
extern void func_8000309C(void *handle, u8 volume);
extern void trackMakePolylist(s32 count, Objects0BB84Vec3 *start,
                              Objects0BB84Vec3 *end, f32 *radius, s32 arg4,
                              s32 arg5);
extern s32 func_80010900(Objects0BB84Vec3 *start, Objects0BB84Vec3 *end,
                         f32 radius, s32 actor, void *callback);
extern u32 func_8001357C(f32 arg0, f32 arg1, f32 *arg2, s32 arg3,
                         void *arg4);
extern void partUpdateTriggers(void *object, s32 updateRate);
extern void func_8000BB84(s32 arg0, Objects0BB84Vec3 *arg1,
                          Objects0BB84Vec3 *arg2, f32 arg3,
                          Objects0BB84Plane *arg4, Objects0BB84Object *arg5);
extern f32 D_8008152C;
extern f32 D_80081530;
extern f32 D_80081534;

typedef struct {
    u8 pad00[0x40];
    Objects58C0Data *unk40;
    u8 pad44[0x24];
    s32 *unk68;
} Objects06448Arg;

typedef struct {
    u8 pad00[8];
    s16 unk8;
    u8 pad0A[0x35];
    u8 unk3F;
} Objects08028Model;

typedef struct {
    u8 pad00[0x40];
    Objects58C0Data *unk40;
    u8 pad44[0x24];
    Objects08028Model **unk68;
} Objects08028Object;

typedef struct {
    u8 pad00[0x1E];
    s8 unk1E;
    u8 pad1F[0x35];
    f32 unk54;
    f32 unk58;
    u8 pad5C[5];
    u8 unk61;
    u8 unk62;
    u8 unk63;
    s16 unk64;
    u16 unk66;
} Objects069E8Source;

typedef struct {
    f32 unk0;
    f32 unk4;
    s32 unk8;
    u16 unkC;
    u16 unkE;
    u8 unk10;
    u8 unk11;
    u8 unk12;
    u8 unk13;
    u8 pad14[8];
    s32 unk1C;
} Objects069E8Target;

typedef struct {
    u8 pad00[0x40];
    Objects069E8Source *unk40;
    u8 pad44[8];
    Objects069E8Target *unk4C;
} Objects069E8Object;

typedef struct {
    u8 pad00[0xA6];
    u8 unkA6;
    u8 padA7;
    u8 *unkA8;
    s32 *unkAC;
} Objects04B04Object;

typedef struct {
    s32 unk0;
    s32 unk4;
} Objects06868Entry;

typedef struct {
    u8 pad00[0x25];
    s8 unk25;
    u8 pad26[0x1E];
    Objects06868Entry *unk44;
} Objects06868Data;

typedef struct {
    u8 pad00[0x40];
    Objects06868Data *unk40;
    u8 pad44[0x28];
    ParticleTrigger *unk6C;
} Objects06868Object;

typedef struct {
    u8 pad00[0x14];
    u16 unk14;
} Objects0A244Header;

typedef struct {
    u8 pad00[0x40];
    Objects0A244Header *unk40;
} Objects0A244Object;

typedef struct {
    u8 pad00[0xC];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x2C];
    s16 unk44;
    u8 pad46[0x4B];
    u8 unk91;
} Objects04454Object;

typedef struct {
    u8 pad00[0x1B];
    u8 unk1B;
} Objects0471CData;

typedef struct {
    u8 pad00[0xC];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x28];
    Objects0471CData *unk40;
    u8 pad44[0x4D];
    u8 unk91;
} Objects0471CObject;

typedef struct {
    u8 pad00[0x1E];
    s8 unk1E;
} Objects06B04Source;

typedef struct {
    s32 unk0;
    s32 unk4;
} Objects0831CCommand;

typedef struct {
    u8 pad00[3];
    u8 unk3;
    u8 pad04[0xA];
    u16 unkE;
    u16 unk10;
    u8 pad12[0xE];
} Objects07C68Texture;

typedef struct {
    s16 unk0;
    s16 unk2;
    s32 unk4;
} Objects07C68Record;

typedef struct {
    u8 pad00[0x18];
    void **unk18;
    u8 pad1C[0x10];
    u8 unk2C;
} Objects07C68Source;

typedef struct {
    u8 pad00[0xA];
    s16 unkA;
    u8 pad0C[0x40];
    Objects07C68Record *unk4C;
    u8 pad50[0x40];
    u8 unk90;
} Objects07C68Object;

typedef struct {
    u8 pad00[0x50];
    s16 *unk50;
} Objects07C68Indexed;

typedef struct {
    u16 unk0;
    s8 unk2;
    u8 unk3;
    u16 unk4;
    u16 unk6;
    f32 unk8;
} Objects06B04Entry;

typedef struct {
    u8 pad00[0x2F];
    u8 unk2F;
    u8 pad30[8];
    u8 *unk38;
} Objects06B04Asset;

typedef struct {
    u8 pad00[8];
    f32 unk8;
    u8 pad0C[0x34];
    Objects06B04Source *unk40;
    u8 pad44[4];
    void *unk48;
    u8 pad4C[0x1C];
    Objects06B04Asset ***unk68;
} Objects06B04Object;

typedef struct {
    u8 pad00[0xA];
    s16 unkA;
    u8 pad0C[0x68];
    u8 *unk74;
} Objects06B04Output;

typedef struct {
    f32 unk0;
    u8 pad04[0x10];
    u16 unk14;
    u8 pad16[6];
    s16 unk1C;
    u8 pad1E[4];
    s8 unk22;
    u8 pad23[0x11];
    s32 *unk34;
} Objects06C4CAsset;

typedef struct {
    u8 pad00[6];
    s16 unk6;
    f32 unk8;
    u8 pad0C[0x20];
    s16 unk2C;
    u8 pad2E[0x12];
    Objects06C4CAsset *unk40;
    u8 pad44[2];
    s16 unk46;
    u8 pad48[0x20];
    s32 *unk68;
    u8 pad6C[0x28];
    s32 unk94[1];
} Objects06C4CObject;

extern void *func_8000486C(s32 arg0);
extern void *func_80006C4C(s32 arg0);
extern void *func_8001F520(s32 assetId, s32 flags);
extern void *func_8002B314(s32 size, s32 tag);
extern void *func_800355A0(s32 assetId, s32 flags);

typedef struct {
    u8 pad00[6];
    s16 unk6;
    u8 pad08[4];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x18];
    f32 unk30;
} Objects0A39CObject;

typedef struct {
    f32 pad00[2];
    f32 unk8;
    f32 pad0C[3];
    f32 unk18;
    f32 pad1C[3];
    f32 unk28;
    f32 pad2C[3];
    f32 unk38;
} Objects0A39CMatrix;

typedef struct {
    s32 start;
    s32 end;
} Objects0486CTableEntry;

typedef struct {
    f32 unk0;
    u8 pad04[0x18];
    s16 unk1C;
    u8 pad1E[0x16];
    s32 unk34;
    s32 unk38;
    s32 unk3C;
    s32 unk40;
    s32 unk44;
    u8 pad48[4];
    s32 unk4C;
    s32 unk50;
    u8 pad54[0x52];
    u8 unkA6;
    u8 padA7;
    s32 unkA8;
    s32 unkAC;
    s32 unkB0;
    u8 padB4[0x2C];
    s32 unkE0;
} Objects0486CAsset;

typedef struct {
    u16 unk0;
    s8 unk2;
    u8 unk3;
    s16 unk4;
    u8 pad06[2];
    s32 unk8;
} Objects06534Record;

typedef struct {
    u8 pad00[0x23];
    s8 unk23;
    s8 unk24;
    u8 pad25[0x13];
    s32 *unk38;
    s32 unk3C;
    Objects06534Record *unk40;
} Objects06534Data;

typedef struct {
    u8 pad00[8];
    f32 unk8;
    u8 pad0C[0x34];
    Objects06534Data *unk40;
    u8 pad44[0x18];
    void *unk5C;
    void *unk60;
    u8 pad64[0x28];
    s8 unk8C;
} Objects06534Object;

typedef struct {
    void *unk0;
    s8 unk4;
    u8 unk5;
    u8 pad06[2];
    f32 unk8;
    s32 unkC;
    f32 unk10;
} Objects06534Sprite;

typedef struct {
    u8 unk0;
    u8 pad01[7];
    s16 unk8;
    u8 pad0A[0x0E];
    s16 unk18;
} Objects07E40Group;

typedef struct {
    u8 pad00[4];
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
} Objects07E40Record;

typedef struct {
    u8 pad00[0x10];
    u8 unk10;
    u8 pad11[5];
    s16 unk16;
    u8 *unk18;
    u8 pad1C[4];
    u8 *unk20;
    u8 *unk24;
} Objects07E40Inner;

typedef struct {
    Objects07E40Inner *unk0;
} Objects07E40Outer;

typedef struct {
    u8 pad00[0x22];
    s8 unk22;
    u8 pad23[0x7F];
    u8 unkA2;
    u8 unkA3;
    s8 unkA4;
    s8 unkA5;
} Objects07E40Data;

typedef struct {
    u8 pad00[0x40];
    Objects07E40Data *unk40;
    u8 pad44[0x24];
    Objects07E40Outer **unk68;
} Objects07E40Object;

typedef struct {
    s16 pad00;
    s16 pad02;
    s16 pad04;
    s16 unk6;
    s16 unk8;
} Objects07E40Texture;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
} Objects08128Bounds;

typedef struct {
    u8 pad00[0x20];
    s16 unk20;
    s16 unk22;
    s16 unk24;
    s16 unk26;
    s16 unk28;
    s16 unk2A;
} Objects08128Track;

typedef struct {
    u8 pad00[0x0C];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x16];
    s16 unk2E;
} Objects08128Object;

extern Objects08128Track *trackGetTrack(void);
extern Objects08128Bounds *func_8000FEEC(s32);
extern s32 func_8000FAE0(f32, f32, f32);

typedef struct {
    u8 pad00[0xA6];
    u8 unkA6;
    u8 padA7;
    u8 *unkA8;
    void **unkAC;
} Objects09220Data;

typedef struct {
    u8 pad00[8];
    f32 unk8;
    u8 pad0C[0x34];
    Objects09220Data *unk40;
} Objects09220Object;

typedef struct {
    u8 pad00[0x26];
    s16 unk26;
    s16 unk28;
    s16 unk2A;
} Objects09220Source;

typedef struct {
    f32 pad00[2];
    f32 unk8;
    f32 pad0C[3];
    f32 unk18;
    f32 pad1C[3];
    f32 unk28;
} Objects09220Matrix;

typedef struct {
    u32 w0;
    u32 w1;
} Objects09220Gfx;

extern void *camGetRotationMtx(void);
extern void *camGetProjOrgMtx(void);
extern void mathOneFloatPY(void *, f32 *);
extern void mtxf_transform_point(void *, f32, f32, f32, f32 *, f32 *, f32 *);
extern s32 func_800246B0(f32, f32, f32, f32 *, f32 *, u8);
extern void func_80034DF0(u8, u8, u8, u8, u8, u8);
extern void func_80034E48(void);
extern void func_80023598(void **, void *, void *, void *, void *, s32);
extern void func_80023A08(void **, s32, s32, s16 *, s32, s32, s32);
extern f32 sqrtf(f32);
extern f32 D_80080F80;
extern f32 D_80080F7C;

typedef struct {
    u8 pad00[0x1C];
    s16 unk1C;
} Objects0A830Data;

typedef struct {
    u8 pad00[0x40];
    Objects0A830Data *unk40;
    u8 pad44[0x20];
    void *unk64;
} Objects0A830Object;

extern void *D_800C94D8[];
extern s32 D_800C9470;
extern s32 D_800C9474;
extern s32 D_800C94A8;
extern s32 D_800C94AC;
extern s32 *D_800C9450;
extern s32 D_800C9454;
extern s32 D_800C945C;
extern u16 D_8007BF1C;
extern void *D_800C94A0;
extern Objects04B04Object **D_800C9488;
extern u16 *D_800C948C;
extern s32 *D_800C94A4;
extern s32 D_800C94B4;
extern s32 D_800C94B8;
extern s32 D_800C94BC;
extern s32 D_8007A210;
extern void *D_8007A214;
extern void *D_8007A218;
extern s32 D_8007A21C;
extern s16 D_800C9508[];
extern s16 D_800C94B0;
extern s16 D_800C94B2;
extern s32 D_80078F84;
extern Objects04454Object *D_80078F20;
extern s8 D_80078F88;
extern s8 D_80079004;
extern u8 D_8007BDA0;
extern u8 D_8007BEF8;
extern u8 D_8007BF0C;
extern s8 D_80079250;
extern f32 D_80080F84;
extern void **D_800C94F4;
extern s32 D_800C94F8;
extern void **D_800C9494;
extern s32 D_800C9498;
extern s32 D_800C949C;
extern s32 D_800C94FC;
extern s32 D_800C9490;
extern u8 *D_800C9460;
extern s32 D_800C9468;
extern s32 *D_800C9458;
extern s16 *D_800C94E0;
extern s32 D_800C94C0[];
extern s32 D_800C94C8[];
extern s32 D_800C94D0[];
extern s32 D_800C94D4[];
extern s32 D_800C9478;
extern f32 D_800C946C;
extern s32 D_80078F80;
extern s32 D_80078F78;
extern s32 D_800C94E8;
extern u8 D_800D3128[];
extern void **D_800C94EC;
extern s32 D_800C94F0;
extern void **D_800C9500;
extern s32 D_800C9504;
extern s32 D_8007A1F4;
extern s32 D_8007A1F8;
extern s32 D_8007A1FC;
extern u8 D_8007BEFC;
extern u8 D_8007BF04;
extern u8 D_8007BF10;
extern void *D_80078F7C;
extern s16 D_80078F8C[];
extern s16 D_80078FA0[];
extern s16 D_80078FB4[];
extern s16 D_80078FC8[];
extern s16 D_80078FDC[];
extern s16 D_80078FF0[];
extern s32 D_800C947C;
extern s32 D_800C9480;
extern s32 D_800C9484;
extern s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset, s32 size);
extern s32 *piRomLoad(s32 assetIndex);
extern s32 runlinkDownloadCode(s32 overlayIndex);
extern void *func_8000590C();
extern f32 sqrtf(f32 value);
extern void func_80006FA0(void);
extern void func_80007118();
extern s32 TrapDanglingJump();
extern void mmFree(void *data);
extern void modFreeModel(void *resource);
extern void func_800347A0(void *texture);
extern void func_800359D4(void *sprite);
extern s32 func_8000A6E8(s32 arg0);
extern void *func_8002B280(s32 size, s32 tag);
extern void *func_8002B4C0(void *slots, s32 size);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void func_80009F74(void *object);
extern u8 *levelGetLevel(void);
extern s32 levelGetNumber(void);
extern s32 controlGetPlayerSetup(s16 *arg0, s16 *arg1, s16 *arg2, s16 *arg3);
extern void func_80058250(void);
extern s32 camGetNo(void);
extern Objects09F74Camera *camGetPtr(void);
extern f32 camGetProjZ(f32 x, f32 y, f32 z);
extern u8 *levelGetColourCycling(void);
extern void func_80008B94(void *object);
typedef struct Objects09AA8Object Objects09AA8Object;
extern void func_80009AA8(Objects09AA8Object *object);
extern s32 func_800290A0(void);
extern void func_800367E8(Objects07C68Texture *texture, void *flags, s32 *frame,
                           s32 updateRate);
extern s32 D_80079008[];
extern s32 D_800790D0[];
extern f32 D_80080D24;
extern f32 D_80080D28;
extern void func_8000831C(void *arg0, void *arg1, s32 arg2, void *arg3, s32 arg4,
                          s32 arg5, s32 arg6, s32 arg7, f32 arg8, s32 arg9, s32 arg10);
typedef struct CameraScaledTransform CameraScaledTransform;
typedef struct FxGfx FxGfx;
extern void camPushModelMtx(Gfx **dlist, Mtx **mtx, CameraScaledTransform *transform,
                            f32 scale, f32 scaleY);
extern void camPopModelMtx(Gfx **dlist);
extern void camRestoreModelMtx(Gfx **dlist);
extern void func_80034920(Gfx **dlist);
extern void func_800349A4(FxGfx **dlist, s32 texture, s32 flags, s32 arg3);
extern s32 func_800291FC(void);
extern s32 func_80034448(s32 resourceId, void *output);
extern void func_8005AF14(void *arg0, void *arg1, void *arg2);
extern void func_80019AB8(void *arg0, void *arg1, s32 arg2, s32 arg3);
extern void func_80007C68(Objects07C68Object *arg0, Objects07C68Source *arg1,
                          Objects07C68Object *arg2, s32 arg3);
extern void **func_8000572C(s32 *start, s32 *end);
extern f32 func_8000BD0C(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5);
extern void partInitTrigger(ParticleTrigger *trigger, s32 type, s32 value);
extern void partInitTriggerSPPos(ParticleTrigger *trigger, s32 type, s32 value, s32 index);
extern void partInitTriggerPos(ParticleTrigger *trigger, s32 type, s32 value, s16 x, s16 y, s16 z);
extern u8 *func_80028F54(void);

void func_80004340(void) {
    D_800C9470 = 0;
    D_800C9474 = 0;
    D_800C9498 = 0;
    D_800C949C = 0;
    D_800C94A8 = 0;
    D_800C94AC = 0;
    D_800C94F0 = 0;
    D_800C94F8 = 0;
    D_800C9504 = 0;
    D_800C94B0 = 0;
    D_800C94B2 = 0;
}
void func_8000439C(void) {
    s32 count;
    s32 i;
    s32 offset;

    D_800C94FC = 0;
    func_80006FA0();
    count = D_800C9498;
    i = 0;
    offset = 0;
    if (count > 0) {
        do {
            func_80007118(*(void **)((u8 *)D_800C9494 + offset), 1);
            i += 1;
            offset += 4;
        } while (i != count);
    }
    D_800C94F0 = 0;
    D_800C9498 = 0;
    D_800C949C = 0;
    func_80004340();
    mmFree(D_800C94D8[0]);
    mmFree(D_800C94D8[1]);
    D_80078F84 = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80004454.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80004590.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000471C.s")
void func_8000485C(s8 arg0) {
    D_80078F88 = arg0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000486C.s")
void func_80004B04(s32 arg0)
{
  s32 temp_v0;
  s32 var_s1;
  s32 var_s2;
  s32 var_s3;
  u16 *temp_v1;
  u16 temp_a1;
  u16 temp_v0_2;
  Objects04B04Object *temp_s0;
  temp_v0 = arg0 * 2;
  temp_v1 = (u16 *) (((u8 *) D_800C948C) + temp_v0);
  temp_a1 = *temp_v1;
  if (temp_a1)
  {
    *temp_v1 = temp_a1 - 1;
    if ((*((u16 *) (((u8 *) D_800C948C) + temp_v0))) == 0)
    {
      var_s2 = 0;
      temp_s0 = D_800C9488[arg0];
      var_s3 = 0;
      var_s1 = 0;
      if (temp_s0->unkA6 > 0)
      {
        do
        {
          ;
          if (((*((u16 *) (temp_s0->unkA8 + var_s3))) & 0xC000) == 0xC000)
          {
            func_800347A0(*((void **) (((u8 *) temp_s0->unkAC) + var_s1)));
          }
          else
            if ((*((u16 *) (temp_s0->unkA8 + var_s3))) & 0x8000)
          {
            func_800359D4(*((void **) (((u8 *) temp_s0->unkAC) + var_s1)));
          }
          else
          {
            modFreeModel(*((void **) (((u8 *) temp_s0->unkAC) + var_s1)));
          }
          var_s2 += 1;
          var_s3 += 2;
          var_s1 += 4;
        }
        while (var_s2 < ((s32) temp_s0->unkA6));
      }
      mmFree(temp_s0);
    }
  }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80004C28.s")
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    u8 pad0A[2];
    s16 unkC;
    s16 unkE;
    u8 unk10;
    u8 unk11;
} Objects04FE0Packet;

typedef struct {
    u8 pad00[4];
    u8 unk4;
    u8 pad05;
    s8 unk6;
    u8 pad07[0x21];
} Objects04FE0ModeRecord;

typedef struct {
    u8 pad00[4];
    s16 unk4;
    s16 unk6;
    s16 unk8;
} Objects04FE0Source;

typedef struct {
    s16 unk0;
    u8 pad02[0x3A];
    void *unk3C;
    u8 pad40[4];
    s16 unk44;
    u8 pad46[0x3E];
    s32 unk84;
    s32 unk88;
} Objects04FE0Object;

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80004FE0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80005548.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_800056A4.s")
void *func_800056F0(s32 index) {
    if ((index < 0) || (index >= D_800C9498)) {
        return 0;
    }
    return D_800C9494[index];
}
void **func_8000572C(s32 *start, s32 *end) {
    *start = D_800C949C;
    *end = D_800C9498;
    return D_800C9494;
}
void **func_80005750(s32 *count) {
    *count = D_800C94F8;
    return D_800C94F4;
}
void func_80005768(AnimPathObject *object) {
    D_800C9500[D_800C9504] = object;
    D_800C9504 += 1;
}
void func_80005798(void *object) {
    s32 i;

    if (D_800C9504 > 0) {
        for (i = 0; i < D_800C9504; i++) {
            if (D_800C9500[i] == object) {
                D_800C9500[i] = D_800C9500[D_800C9504 - 1];
            }
        }
        D_800C9504 -= 1;
    }
}
void **func_80005808(s32 *count) {
    *count = D_800C9504;
    return D_800C9500;
}
s32 func_80005820(s32 arg0) {
    if (D_800C94F8 == 0) {
        return 0;
    }
    if ((arg0 < 0) || (arg0 >= D_800C94F8)) {
        return 0;
    }
    return D_800C94F4[arg0];
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80005868.s")
s8 func_800058C0(Objects58C0Arg *arg0, s32 arg1) {
    if ((arg1 >= 4) || (arg0->unk40->unkD0[arg1] == 0.0f)) {
        return arg0->unk40->unk1E[0];
    }
    return arg0->unk40->unk1E[arg1];
}
typedef struct {
    s16 unk0;
    u8 pad02[2];
    s16 unk4;
    s16 unk6;
    s16 unk8;
} Objects0590CPacket;

typedef struct {
    f32 unk0;
    u8 pad04[0x10];
    u16 unk14;
    u8 pad16[2];
    s16 unk18;
    s8 unk1A;
    u8 pad1B[1];
    s16 unk1C;
    u8 pad1E[4];
    s8 unk22;
    s8 unk23;
    s8 unk24;
    s8 unk25;
    u8 pad26[2];
    s8 unk28;
    u8 unk29;
    u8 pad2A[6];
    u8 unk30;
    u8 pad31[3];
    s32 *unk34;
    u8 pad38[0x29];
    u8 unk61;
    u8 pad62[0x10];
    s8 unk72;
    u8 pad73[0x34];
    u8 unkA7;
    u8 padA8[0xC];
    s16 unkB4;
    s16 unkB6;
    s16 unkB8;
    u8 unkBA;
    u8 unkBB;
    f32 unkBC;
    f32 unkC0;
    f32 unkC4;
    f32 unkC8;
    s32 unkCC;
    u8 padD0[0x10];
    s32 unkE0;
} Objects0590CAsset;

typedef struct {
    u8 pad00[0x30];
    s32 unk30;
} Objects0590CParticleList;

typedef struct Objects0AA38Object Objects0AA38Object;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x14];
    s16 unk2C;
    s16 unk2E;
    u8 pad30[4];
    f32 unk34;
    u8 pad38[1];
    u8 unk39;
    u8 pad3A[2];
    void *unk3C;
    Objects0590CAsset *unk40;
    u8 pad44[2];
    s16 unk46;
    s32 unk48;
    s32 unk4C;
    s32 unk50;
    s32 unk54;
    s32 unk58;
    s32 unk5C;
    s32 unk60;
    s32 unk64;
    s32 *unk68;
    s32 unk6C;
    s32 unk70;
    s32 unk74;
    s32 unk78;
    s32 unk7C;
    u8 pad80[0xD];
    u8 unk8D;
    u8 pad8E[3];
    u8 unk91;
    u8 unk92;
    u8 unk93;
    s32 unk94[1];
} Objects0590CObject;

extern u8 *align4(u8 *address);
extern void lightSetupLightSources(void *object);
extern void lightSetupFlareSources(void *object);
extern s32 func_8001A008(void *object, void *state);
extern void modelSetModelFlags(s32 flags);
extern s32 func_80048760(void *object, s32 state);
extern void func_80053550(void *source, s32 kind, s32 mode, s16 rotationX,
                           s16 rotationY, s16 rotationZ, f32 radius, f32 height,
                           f32 arg8, f32 arg9, s32 collisionType, u16 flags);
extern void func_8000AA38(Objects0AA38Object *object, void *arg1);
extern void func_80006448(void *object);
extern s32 func_80006534(Objects06534Object *object);
extern s32 func_80006868(Objects06868Object *object, void *data);
extern s32 func_800069C0(Objects69C0In *object, Objects69C0Out *data);
extern s32 func_800069E8(Objects069E8Object *object, Objects069E8Target *data);
extern s32 func_80006B04(Objects06B04Object *object, Objects06B04Output *data,
                          volatile s32 arg2);
extern s32 func_80006C40(Objects06C40 *object, s32 data);
extern s32 func_8000A830(Objects0A830Object *object, void *data);

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000590C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006448.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006534.s")
/* PROVENANCE: control-flow body adapted from Jet Force Gemini's public
 * src/objects.c func_80007494; Mickey's offsets, globals, and calls are authoritative. */
s32 func_80006868(Objects06868Object *arg0, void *arg1)
{
  s32 temp_v0_2;
  s32 temp_v1;
  s32 temp_v1_2;
  s32 var_s1;
  s32 var_s3;
  s8 var_v1;
  Objects06868Entry *temp_a2;
  Objects06868Data *temp_v0;
  Objects06868Entry *var_s0;
  temp_v0 = arg0->unk40;
  arg0->unk6C = arg1;
  var_v1 = temp_v0->unk25;
  var_s3 = 0;
  temp_a2 = temp_v0->unk44;
  if (var_v1 <= 0)
  {
    goto done;
  }
  var_s0 = temp_a2;
  var_s1 = 0;
  do
  {
    temp_v0_2 = var_s0->unk0;
    temp_v1 = temp_v0_2 & 0xFFFF0000;
    if (temp_v1 == 0xFFFF0000)
    {
      partInitTrigger(((u8 *) arg0->unk6C) + var_s1, (temp_v0_2 >> 8) & 0xFF, temp_v0_2 & 0xFF);
    }
    else
      if ((temp_v0_2 & 0xFFFF0000) == 0xFFFE0000)
    {
      partInitTriggerSPPos(((u8 *) arg0->unk6C) + var_s1, (temp_v0_2 >> 8) & 0xFF, temp_v0_2 & 0xFF, var_s0->unk4 & 0xFF);
    }
    else
    {
      temp_v1_2 = var_s0->unk4;
      partInitTriggerPos(((u8 *) arg0->unk6C) + var_s1, (temp_v0_2 >> 24) & 0xFF, (temp_v0_2 >> 16) & 0xFF, temp_v0_2 & 0xFFFF, (temp_v1_2 >> 16) & 0xFFFF, temp_v1_2 & 0xFFFF);
    }
    var_s3 += 1;
    var_s1 += 0x24;
    var_v1 = arg0->unk40->unk25;
    var_s0 += 1;
  }
  while (var_s3 < var_v1);
  done:
  return ((var_v1 * 0x24) + 3) & (~3);

}
s32 func_800069C0(Objects69C0In *arg0, Objects69C0Out *arg1) {
    arg0->unk78 = arg1;
    arg1->unk4 = arg0->unk40->unkE0->unk2C;
    arg0->unk78->unk0 = 2;
    return 0x2C;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_800069E8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006B04.s")
s32 func_80006C40(Objects06C40 *arg0, s32 arg1) {
    arg0->unk58 = arg1;
    return 0x13C;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006C4C.s")
void func_80006EA0(void *ptr) {
    if (((u8 *) ptr)[0x91] == 0) {
        ((u8 *) ptr)[0x91] = 1;
        D_800C94EC[D_800C94F0] = ptr;
        D_800C94F0 += 1;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006EE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80006FA0.s")
extern void func_800031E8(void *object);
extern void func_80005798(void *object);
extern void func_8000D728(void *object);
extern void camlightDelete(void *object);
extern void partObjFreeTriggers(void *object);
extern void partNullifyCircularParticleParents(void *object);
extern void lightKillGlowingLight(void);
extern void func_80048980(void *object);
extern void func_8001C088(void *object);
extern void killLight(void *light);
extern void amSndStop(void *sound);
extern void func_80046E70(void *object);

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80007118.s")
void func_80007844(void) {
}
typedef struct {
    u8 pad00[0x1E];
    s8 unk1E;
    u8 pad1F[0x82];
    u8 unkA1[1];
    u8 unkA2;
} Objects0784CData;

typedef struct {
    u8 pad00[0x38];
    f32 unk38;
    f32 unk3C;
    f32 unk40;
    u8 pad44[0xB2];
    s16 unkF6;
    s16 unkF8;
    s16 unkFA;
} Objects0784COutput;

typedef struct {
    u8 pad00[8];
    void *unk8;
    u16 unkC;
    u16 unkE;
} Objects0784CAnimation;

typedef struct {
    void *unk0;
    u8 pad04[1];
    u8 unk5;
    u8 pad06[6];
    s32 unkC;
    u8 pad10[4];
} Objects0784CEffect;

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    u8 pad06[6];
    f32 unkC;
    f32 unk10;
    f32 unk14;
    u8 pad18[0x28];
    Objects0784CData *unk40;
    s16 unk44;
    u8 pad46[6];
    Objects0784CAnimation *unk4C;
    u8 pad50[4];
    void *unk54;
    u8 pad58[8];
    Objects0784CEffect *unk60;
    Objects0784COutput *unk64;
    u8 pad68[0x24];
    u8 unk8C;
} Objects0784CObject;

extern s32 runlinkIsModuleLoaded(s32 module);
extern void func_8000AEEC(void *object, s32 updateRate);
extern void func_8000B3CC(void *object, s32 updateRate);
extern void spranimOnceControl(void *object, s32 updateRate);
extern void spranimControl(void *object, s32 updateRate);
extern void texscrollControl(void *object, s32 updateRate);
extern void effectboxControl(void *object, s32 updateRate);
extern void func_800148E0(void *object);
extern void func_8001B798(void *object, s32 updateRate);
extern void func_8001BB04(void *object, s32 updateRate);
extern void func_8001BB10(void *object, s32 updateRate);
extern void rangetriggerControl(void *object, s32 updateRate);
extern void func_80007E40(Objects07E40Object *object, s32 updateRate);
extern void func_80049000(void *object, s32 updateRate);
extern void func_80036544(void *entry, s32 *mode, s32 animationId, void *state,
                          s32 updateRate);
extern void func_8001CB84(void *object, s32 updateRate);
extern void func_8001D2A0(void *object, s32 updateRate);
extern void func_80053868(s32 updateRate);
extern void lightUpdateLights(s32 updateRate);
extern void lightUpdateObjects(void);
extern void amPlayAudioMap(void **objects, s32 count, s32 updateRate);

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000784C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80007C68.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80007E40.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80008028.s")
void func_80008118(void) {
    D_80079004 = 1;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80008128.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000831C.s")
typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Objects084C4Point;

typedef struct {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Objects084C4Vertex;

typedef struct {
    u32 w0;
    u32 w1;
} Objects084C4Gfx;

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_800084C4.s")
void func_80008A20(Objects08A20Arg *arg0) {
    func_8000831C(arg0, D_80079008, 0x14, D_800790D0, 0x18, *arg0->unk68, 2, 0, 1.0f, 0xFF, 0xFF);
}
void func_80008A8C(Objects08A20Arg *arg0) {
    switch (arg0->unk44) {
        case 0x3D:
            if (arg0->unk88 != NULL) {
                TrapDanglingJump(arg0);
            }
            TrapDanglingJump(&D_800C94B4, arg0);
            break;
        case 0x24:
            TrapDanglingJump(&D_800C94B4, &D_800C94B8, arg0);
            break;
        case 0x3B:
            func_80008A20(arg0);
            break;
        case 0x41:
            TrapDanglingJump(arg0, &D_800C94B4, &D_800C94B8, &D_800C94BC);
            break;
        case 0x55:
            TrapDanglingJump(&D_800C94B4, &D_800C94B8, arg0);
            break;
        case 0x5B:
            TrapDanglingJump(&D_800C94B4, &D_800C94B8, arg0);
            break;
    }
}

typedef struct {
    u8 pad00[0x1E];
    u8 unk1E;
    u8 pad1F[0x11];
    u8 unk30;
} Objects08B94Data;

typedef struct {
    u8 pad00[6];
    s16 unk6;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
} Objects08B94Resource;

typedef struct {
    f32 unk0;
    u8 pad04;
    u8 unk5;
    u8 unk6;
    u8 unk7;
} Objects08B94Multiplier;

typedef struct {
    u8 pad00[0xD];
    u8 unkD;
} Objects08B94Palette;

typedef struct {
    u8 pad00[8];
    u8 r;
    u8 g;
    u8 b;
    u8 pad0B[5];
} Objects08B94Colour;

typedef struct {
    u8 pad00[0x64];
    u8 *unk64;
} Objects08B94Camera;

typedef struct {
    u8 pad00[4];
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
    u8 unk9;
    u8 pad0A[0xA];
    Objects08B94Camera *unk14;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
} Objects08B94Info;

typedef struct {
    u8 pad00[6];
    s16 unk6;
    f32 unk8;
    f32 unkC;
    f32 unk10;
    s32 unk14;
    u8 pad18[0x21];
    u8 unk39;
    s8 unk3A;
    u8 pad3B;
    Objects08B94Palette *unk3C;
    Objects08B94Data *unk40;
    s16 unk44;
    u8 pad46[0xA];
    Objects08B94Multiplier *unk50;
    u8 pad54[0x10];
    Objects08B94Info *unk64;
    Objects08B94Resource **unk68;
} Objects08B94Object;

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80008B94.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80009220.s")
typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Objects09414Vector;

typedef struct {
    u8 pad00[0xA];
    s16 unkA;
    u8 pad0C[0x34];
    Objects09414Vector *unk40;
    u8 pad44[0xC];
    f32 unk50;
    u8 pad54[0x104];
    s16 unk158;
} Objects09414Resource;

typedef struct {
    f32 unk0;
    u8 pad04[0x1A];
    s8 unk1E[4];
} Objects09414Data;

typedef struct {
    void *unk0;
    s8 unk4;
    u8 pad05[3];
    f32 unk8;
    u8 pad0C[8];
} Objects09414Entry;

typedef struct {
    u8 unk0;
    u8 pad01;
    s8 unk2;
    u8 unk3;
    u8 unk4;
    u8 pad05[3];
    void *unk8;
} Objects09414StaticEntry;

typedef struct {
    s16 angle;
    s16 frame;
    u16 pad04;
    u16 divisor;
    f32 transformScale;
    f32 matrixScale;
    f32 x;
    f32 y;
    f32 z;
    s32 frameCount;
    void *spriteData;
} Objects09414Sprite;

typedef struct {
    u32 w0;
    u32 w1;
} Objects09414Gfx;

typedef struct {
    u8 pad00[0x39];
    u8 unk39;
    u8 pad3A[6];
    Objects09414Data *unk40;
    u8 pad44[0xC];
    f32 *unk50;
    u8 pad54[0xC];
    Objects09414Entry *unk60;
    Objects09414Resource *unk64;
    Objects09414Resource **unk68;
    u8 pad6C[0x20];
    u8 unk8C;
    u8 pad8D[6];
    u8 unk93;
} Objects09414Object;

extern void func_80022E80(void *transform);
extern void func_80022FD4(void **displayList, s32 matrices, s32 vertices,
                          void *transform, f32 *opacity,
                          Objects09414Sprite *sprite, s32 flags, s32 alpha);
extern void func_80047CD8(void **displayList, void *cone, s32 flags, u8 alpha);
extern f32 func_80009F08(Objects09F08Arg *arg0);

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80009414.s")
typedef struct {
    u8 pad00[0xD4];
    f32 unkD4;
} Objects09AA8Data;

typedef struct {
    u8 pad00[0x11];
    u8 unk11;
    u8 pad12[0x3C];
    s8 unk4E;
    u8 pad4F[0x21];
    s32 unk70;
    u8 *unk78;
} Objects09AA8Material;

typedef struct {
    void *unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    u8 pad10[0x40];
    s32 unk50;
} Objects09AA8Entry;

typedef struct {
    void *unk0;
    u8 pad04[0x64];
    s32 unk68;
    s32 unk6C;
} Objects09AA8Root;

struct Objects09AA8Object {
    u8 pad00[0x39];
    u8 unk39;
    s8 unk3A;
    u8 pad3B[5];
    Objects09AA8Data *unk40;
    u8 pad44[0xC];
    s32 unk50;
    u8 pad54[0x14];
    Objects09AA8Entry **unk68;
    u8 pad6C[0x27];
    s8 unk93;
};

typedef struct {
    u32 w0;
    u32 w1;
} Objects09AA8Command;

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80009AA8.s")
void func_80009E78(Gfx **displayList, Mtx **matrix, TrackVertex **vertices,
                   TrackSkyObject *object) {
    if ((object->flags & 0xC00) == 0) {
        D_800C94B4 = (s32) *displayList;
        D_800C94B8 = (s32) *matrix;
        D_800C94BC = (s32) *vertices;
        func_80009F74(object);
        *displayList = (Gfx *) D_800C94B4;
        *matrix = (Mtx *) D_800C94B8;
        *vertices = (TrackVertex *) D_800C94BC;
    }
}
f32 func_80009F08(Objects09F08Arg *arg0) {
    f32 temp_f0;
    f32 var_f2;

    var_f2 = 1.0f;
    if (D_8007BF0C == 0) {
        temp_f0 = arg0->unk30;
        if (temp_f0 > 250.0f) {
            var_f2 += (temp_f0 - 250.0f) * 0.00134f;
            if (var_f2 > 2.0f) {
                var_f2 = 2.0f;
            }
        }
    }
    return var_f2;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_80009F74.s")
/* PROVENANCE: partition loop adapted from Diddy Kong Racing's public
 * src/objects.c get_first_active_object; Mickey's list and header offsets are authoritative. */
s32 func_8000A244(s32 *arg0) {
    s32 i;
    s32 j;
    s32 minIndex;
    s32 maxIndex;
    s32 breakLoop;
    Objects0A244Object *tempObject;

    *arg0 = D_800C9498;
    if (D_800C94B2 != 0) {
        return D_800C94B2;
    }
    i = D_800C949C;
    j = D_800C9498 - 1;
    minIndex = i;
    maxIndex = j;
    while (i <= j) {
        breakLoop = 0;
        while ((i <= maxIndex) && (breakLoop == 0)) {
            if (((Objects0A244Object **)D_800C9494)[i]->unk40->unk14 & 1) {
                i += 1;
            } else {
                breakLoop = -1;
            }
        }
        breakLoop = 0;
        while ((j >= minIndex) && (breakLoop == 0)) {
            if (!(((Objects0A244Object **)D_800C9494)[j]->unk40->unk14 & 1)) {
                j -= 1;
            } else {
                breakLoop = -1;
            }
        }
        if (i < j) {
            tempObject = ((Objects0A244Object **)D_800C9494)[i];
            ((Objects0A244Object **)D_800C9494)[i] = ((Objects0A244Object **)D_800C9494)[j];
            ((Objects0A244Object **)D_800C9494)[j] = tempObject;
            i += 1;
            j -= 1;
        }
    }
    D_800C94B2 = i;
    return i;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000A39C.s")
/* PROVENANCE: body adapted from Jet Force Gemini's public src/objects.c
 * setObjectViewNormal; Mickey's target globals and byte output are authoritative. */
void func_8000A62C(f32 x, f32 y, f32 z) {
    f32 vecLength = sqrtf((x * x) + (y * y) + (z * z));
    f32 normalizedLength;

    if (vecLength != 0.0f) {
        normalizedLength = -8192.0f / vecLength;
        x *= normalizedLength;
        y *= normalizedLength;
        z *= normalizedLength;
    }
    D_800C9508[0] = x;
    D_800C9508[1] = y;
    D_800C9508[2] = z;
}
void func_8000A6DC(s32 arg0) {
    D_800C94B0 = arg0;
}
s32 func_8000A6E8(s32 arg0) {
    s32 flags;

    switch (arg0) {
        case 1:
            flags = 0xF01;
            break;
        case 4:
            flags = 0x301;
            break;
        case 3:
            flags = 0xB01;
            break;
        case 31:
            flags = 0x1101;
            break;
        case 24:
            flags = 0x1301;
            break;
        case 25:
            flags = 0x1200;
            break;
        case 26:
            flags = 0xB01;
            break;
        case 22:
        case 29:
        case 73:
        case 79:
            flags = 0x301;
            break;
        case 23:
            flags = 0x200;
            break;
        case 11:
        case 12:
        case 49:
            flags = 0x200;
            break;
        case 33:
            flags = 0x301;
            break;
        case 41:
            flags = 0x1101;
            break;
        case 53:
            flags = 0x200;
            break;
        case 54:
            flags = 0x101;
            break;
        case 55:
            flags = 0x101;
            break;
        case 56:
            flags = 0x101;
            break;
        case 57:
            flags = 0xB01;
            break;
        case 58:
            flags = 0xA00;
            break;
        case 60:
            flags = 0x200;
            break;
        case 63:
            flags = 0x101;
            break;
        case 64:
            flags = 0xB01;
            break;
        case 65:
            flags = 0x301;
            break;
        case 67:
            flags = 0xB01;
            break;
        case 71:
            flags = 0x101;
            break;
        case 72:
            flags = 0x301;
            break;
        case 74:
            flags = 0x301;
            break;
        case 75:
            flags = 0x101;
            break;
        case 77:
            flags = 0x101;
            break;
        case 78:
            flags = 0x301;
            break;
        case 82:
            flags = 0xB01;
            break;
        case 84:
            flags = 0x301;
            break;
        case 85:
            flags = 0x200;
            break;
        case 86:
            flags = 0x101;
            break;
        case 87:
            flags = 0x301;
            break;
        case 88:
            flags = 0x301;
            break;
        default:
            flags = 0;
            break;
    }
    return flags;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000A830.s")
struct Objects0AA38Object {
    u8 pad00[0x40];
    Objects0A830Data *unk40;
    s16 unk44;
};

extern void func_8001C4C0(void *object, void *arg1, s32 mode);
extern void spranimInit(void *object, void *arg1);
extern void sprasjiInit(void *object, void *arg1);
extern void func_8001A154(void *object);
extern void func_8001BAE4(void *object, void *arg1);
extern void func_8001BAF8(void *object, void *arg1);

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000AA38.s")
typedef struct {
    u8 pad00[0x40];
    u8 *unk40;
    s16 unk44;
    u8 pad46[0x32];
    void *unk78;
} Objects0AEECObject;

#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000AEEC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000B3CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/objects/func_8000BB84.s")
void GetRomlistInfo(s32 *romlist, s32 *size, s32 index) {
    *romlist = D_800C94C0[index];
    *size = D_800C94C8[index];
}
f32 func_8000BCB0(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5)
{
  f32 temp_f0;
  f32 temp_f16;
  f32 temp_f2;
  temp_f0 = arg0 - arg3;
  temp_f2 = arg1 - arg4;
  temp_f16 = arg2 - arg5;
  return sqrtf(((temp_f0 * temp_f0) + (temp_f2 * temp_f2)) + (temp_f16 * temp_f16));
}
f32 func_8000BD0C(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5)
{
  f32 temp_f16;
  f32 temp_f18;
  f32 temp_f2;
  temp_f2 = arg0 - arg3;
  temp_f16 = arg1 - arg4;
  temp_f18 = arg2 - arg5;
  return ((temp_f2 * temp_f2) + (temp_f16 * temp_f16)) + (temp_f18 * temp_f18);
}
