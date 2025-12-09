#ifndef __HUD_H__
#define __HUD_H__

#include "gf2d_sprite.h"
#include "gfc_vector.h"
#include "gfc_types.h"
#include "entity.h"

#define SPEED_FRAMES 15   // speed_1.png → speed_15.png

typedef struct HUD {
    Sprite *speedFrames[SPEED_FRAMES];  // All meter frames
    int currentFrame;

    GFC_Vector2D meterPos;
    GFC_Vector2D meterScale;

} HUD;

/**
 * @brief Initialize the HUD system
 * @return pointer to HUD or NULL on error
 */
HUD* hud_init();

/**
 * @brief Update HUD based on player state
 * @param hud the HUD to update
 * @param player the player entity
 */
void hud_update(HUD *hud, Entity *player);

/**
 * @brief Draw the HUD
 * @param hud the HUD to draw
 */
void hud_draw(HUD *hud);

/**
 * @brief Free HUD resources
 * @param hud the HUD to free
 */
void hud_free(HUD *hud);

#endif
