#ifndef __HUD_H__
#define __HUD_H__

#include "gfc_vector.h"
#include "gf2d_sprite.h"
#include "entity.h"

#define SPEED_FRAMES 10
#define FUEL_IDLE_FRAMES 3
#define FUEL_LEAK_FRAMES 10

typedef struct {
    // Speed meter
    Sprite* speedFrames[SPEED_FRAMES];
    GFC_Vector2D meterPos;
    GFC_Vector2D meterScale;
    int currentFrame;
    
    // Fuel gauge
    Sprite* fuelIdleFrames[FUEL_IDLE_FRAMES];
    Sprite* fuelLeakFrames[FUEL_LEAK_FRAMES];
    GFC_Vector2D fuelPos;
    GFC_Vector2D fuelScale;
    int currentFuelFrame;
    Uint8 fuelIsLeaking;  // Flag for which animation to use
    float fuelAnimTimer;  // Timer for idle animation
    
} HUD;

HUD* hud_init();
void hud_update(HUD* hud, Entity* player);
void hud_draw(HUD* hud);
void hud_free(HUD* hud);

#endif