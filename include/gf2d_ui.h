#ifndef __GF2D_UI_H__
#define __GF2D_UI_H__

#include "gf2d_sprite.h"

typedef struct {
    Sprite *sprite;

    GFC_Vector2D position;
    GFC_Vector2D* scale;
    GFC_Vector2D* center;

    float* rotation;

    GFC_Vector2D* flip;
    GFC_Color* colorShift;
    GFC_Vector4D* clip;
    Uint32 frame;

} UI_Element;

void gf2d_ui_draw(UI_Element* ui);

#endif

