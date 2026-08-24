#include "overlays/overlay_090.h"

/* DKR v77/v80 and JFG contain no exact donor for this state initializer. */
void overlay90Initialize(Overlay90Owner *owner, Overlay90Config *config) {
    Overlay90State *state;

    state = owner->state;
    state->active = 1;
    state->flag = 0;
    state->angle = config->angle;
    state->x = config->x;
    state->y = config->y;
    state->z = config->z;
    state->value26 = 0;
    state->value28 = 0;
    state->value14 = 0.0f;
    state->value24 = -0x8000;
    state->value2A = 0;
    state->value34 = 0;
    state->value36 = 0;
    state->value38 = 0;
    state->value3C = 0x7F;
    state->value26 = -0x1F00;
    state->value28 = -0x2040;
    state->value2C = 0x20;
    state->value2E = 0x80;
    state->value10 = 0.0f;
    state->value18 = 0.0f;
    state->value1C = 0.0f;
    state->value20 = 0.0f;
    state->value30 = 0.0f;
    state->value14 = 15.0f;
    state->value1C = gOverlay90Value1C;
    state->value20 = gOverlay90Value20;
    overlay90CommitReloc(owner, 0, -1, 0.0f);
}
