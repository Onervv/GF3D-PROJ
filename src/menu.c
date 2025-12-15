#include <SDL.h>
#include "menu.h"
#include "simple_logger.h"
#include "gf2d_mouse.h"
#include "gf2d_sprite.h" 
#include "gfc_input.h"
#include <stdio.h>

// Hardcoded resolution 
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

Menu* menu_init() {
    Menu* menu = gfc_allocate_array(sizeof(Menu), 1);
    if (!menu) return NULL;
    
    char path[256];

    // Load idle frame 
    // FIX: Added 'mainnohover/' directory to path
    snprintf(path, sizeof(path), "images/ui/mainnohover/mainnohover_1.png");
    menu->idleFrames[0] = gf2d_sprite_load_image(path);
    
    if (!menu->idleFrames[0]) {
        slog("WARNING: Could not load idle frame: %s", path);
    }
    
    // Load hover animation frames
    int hoverLoaded = 0;
    for (int i = 0; i < MENU_HOVER_FRAMES; i++) {
        // FIX: Added 'mainhover/' directory to path
        snprintf(path, sizeof(path), "images/ui/mainhover/mainhover_%d.png", i + 1);
        menu->hoverFrames[i] = gf2d_sprite_load_image(path);
        if (menu->hoverFrames[i]) hoverLoaded++;
    }

    slog("Menu Init: Loaded 1 idle frame and %d hover frames", hoverLoaded);
    
    menu->state = MENU_STATE_MAIN;
    menu->currentFrame = 0;
    menu->animTimer = 0.0f;
    menu->frameDelay = 0.1f;
    menu->isHovering = 0;
    
    // Define start button hitbox 
    menu->startButtonBox = gfc_rect(640 - 200, 360 - 40, 400, 80);
    
    return menu;
}

void menu_update(Menu* menu, float deltaTime) {
    if (!menu) return;
    if (menu->state != MENU_STATE_MAIN) return;
    
    GFC_Vector2D mousePos = gf2d_mouse_get_position();
    
    // 1. Update Hover State
    Uint8 wasHovering = menu->isHovering;
    menu->isHovering = gfc_point_in_rect(mousePos, menu->startButtonBox);
    
    if (wasHovering != menu->isHovering) {
        menu->currentFrame = 0;
        menu->animTimer = 0.0f;
    }
    
    // 2. Update Animation
    menu->animTimer += deltaTime;
    if (menu->animTimer >= menu->frameDelay) {
        menu->animTimer = 0.0f;
        
        if (menu->isHovering) {
            menu->currentFrame++;
            if (menu->currentFrame >= MENU_HOVER_FRAMES || !menu->hoverFrames[menu->currentFrame]) {
                menu->currentFrame = 0;
            }
        } else {
            menu->currentFrame = 0;
        }
    }
    
    // 3. Check Input
    int rawMouseClick = (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT));
    int inputClick = gfc_input_command_pressed("click");
    
    if (menu->isHovering && (inputClick || rawMouseClick)) {
        slog("Start button clicked!");
        menu->state = MENU_STATE_GAME;
    }
}

void menu_draw(Menu* menu) {
    if (!menu) return;
    if (menu->state != MENU_STATE_MAIN) return;
    
    Sprite* currentSprite = NULL;
    
    if (menu->isHovering) {
        if (menu->currentFrame < MENU_HOVER_FRAMES) currentSprite = menu->hoverFrames[menu->currentFrame];
    } else {
        currentSprite = menu->idleFrames[0];
    }
    
    if (!currentSprite) {
         if (menu->isHovering) currentSprite = menu->hoverFrames[0];
         else currentSprite = menu->idleFrames[0];
    }

    if (currentSprite) {
        // --- SCALING AND POSITIONING CONTROLS ---
        
        
        // 1.0 = Original Size, 0.5 = Half Size, etc.
        float scaleVal = 0.5f; 
        
        GFC_Vector2D scale = gfc_vector2d(scaleVal, scaleVal);

        // image is 1280x720 
        float imageW = 1280.0f;
        float imageH = 720.0f;


        GFC_Vector2D pos;

        pos.x =  0.5f;
        pos.y =  0.0f;

        gf2d_sprite_draw(
            currentSprite,
            pos,     // Use calculated position
            &scale,  // Use manual scale
            NULL,    // Center
            NULL,    // Rotation
            NULL,    // Flip
            NULL,    // Color
            NULL,    // Clip
            0        // Frame
        );
    } 
}

void menu_free(Menu* menu) {
    if (!menu) return;
    free(menu);
}

MenuState menu_get_state(Menu* menu) {
    if (!menu) return MENU_STATE_MAIN;
    return menu->state;
}