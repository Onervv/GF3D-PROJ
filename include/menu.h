#ifndef __MENU_H__
#define __MENU_H__

#include "gf2d_sprite.h"
#include "gfc_vector.h"
#include "gfc_types.h"
#include "gfc_shape.h"

#define MENU_IDLE_FRAMES 1   // Number of idle animation frames
#define MENU_HOVER_FRAMES 4  // Number of hover animation frames

typedef enum {
    MENU_STATE_MAIN,
    MENU_STATE_GAME,
    MENU_STATE_OPTIONS,
    MENU_STATE_EXIT
} MenuState;

typedef struct {
    Sprite* idleFrames[MENU_IDLE_FRAMES];   // Idle animation
    Sprite* hoverFrames[MENU_HOVER_FRAMES]; // Hover animation
    
    MenuState state;
    
    int currentFrame;
    float animTimer;
    float frameDelay;  // Time between frames
    
    Uint8 isHovering;  // Is mouse over start button?
    
    // Start button hitbox (center of screen)
    GFC_Rect startButtonBox;
    
} Menu;

Menu* menu_init();
void menu_update(Menu* menu, float deltaTime);
void menu_draw(Menu* menu);
void menu_free(Menu* menu);
MenuState menu_get_state(Menu* menu);

#endif