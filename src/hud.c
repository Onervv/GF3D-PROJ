#include "hud.h"
#include "gf2d_sprite.h"
#include "player.h"
#include "simple_logger.h"

HUD* hud_init() {
    HUD *hud = gfc_allocate_array(sizeof(HUD), 1);
    if (!hud) return NULL;
    
    // Load speed meter frames
    for (int i = 0; i < SPEED_FRAMES; i++) {
        char path[256];
        snprintf(path, sizeof(path), "images/ui/speedometer/speed_%d.png", i + 1);
        hud->speedFrames[i] = gf2d_sprite_load_image(path);
        if (!hud->speedFrames[i]) {
            slog("ERROR loading speed frame %d: %s", i, path);
        }
    }
    
    // Load fuel idle frames
    for (int i = 0; i < FUEL_IDLE_FRAMES; i++) {
        char path[256];
        snprintf(path, sizeof(path), "images/ui/idlefuel/fuel_idle_%d.png", i + 1);
        hud->fuelIdleFrames[i] = gf2d_sprite_load_image(path);
        if (!hud->fuelIdleFrames[i]) {
            slog("ERROR loading fuel idle frame %d: %s", i, path);
        }
    }
    
    // Load fuel leak frames
    for (int i = 0; i < FUEL_LEAK_FRAMES; i++) {
        char path[256];
        snprintf(path, sizeof(path), "images/ui/leakfuel/leak_%d.png", i + 1);
        hud->fuelLeakFrames[i] = gf2d_sprite_load_image(path);
        if (!hud->fuelLeakFrames[i]) {
            slog("ERROR loading fuel leak frame %d: %s", i, path);
        }
    }
    
    // Speed meter position & scale
    hud->meterPos = gfc_vector2d(100, 520);
    hud->meterScale = gfc_vector2d(0.125f, 0.125f);
    hud->currentFrame = 0;
    
    // Fuel gauge position & scale (adjust as needed)
    hud->fuelPos = gfc_vector2d(0, 0);  
    hud->fuelScale = gfc_vector2d(1.0f, 1.0f);
    hud->currentFuelFrame = 0;
    hud->fuelIsLeaking = 0;
    hud->fuelAnimTimer = 0.0f;
    
    return hud;
}

void hud_update(HUD *hud, Entity *player) {
    if (!hud || !player) return;
    
    PlayerData *pd = (PlayerData*)player->data;
    if (!pd) return;
    
    // ===== UPDATE SPEED METER (existing code) =====
    float speed = gfc_vector3d_magnitude(player->velocity);
    float maxSpeed = pd->maxSpeed;
    float normalized = 0.0f;
    
    if (maxSpeed > 0) {
        normalized = speed / maxSpeed;
        if (normalized > 1.0f) normalized = 1.0f;
    }
    
    int baseFrame = (int)(normalized * (SPEED_FRAMES - 1));
    int frame = (SPEED_FRAMES - 1) - baseFrame;
    if (frame < 0) frame = 0;
    if (frame >= SPEED_FRAMES) frame = SPEED_FRAMES - 1;
    hud->currentFrame = frame;
    
    // Determine if boost is active or on cooldown
    if (pd->speedBoostActive || pd->speedBoostCooldown > 0) {
        // LEAKING - show drain animation
        hud->fuelIsLeaking = 1;
        
        // Calculate which leak frame to show based on cooldown/timer
        float fuelPercent;
        
        if (pd->speedBoostActive) {
            // During boost, show timer draining
            fuelPercent = pd->speedBoostTimer / 3.0f;  // Assuming max boost time is 3 seconds
        } else {
            // During cooldown, show refilling
            fuelPercent = 1.0f - (pd->speedBoostCooldown / 5.0f);  // Assuming max cooldown is 5 seconds
        }
        
        // Clamp to 0-1 range
        if (fuelPercent < 0.0f) fuelPercent = 0.0f;
        if (fuelPercent > 1.0f) fuelPercent = 1.0f;
        
        // Map to leak frames (frame 0 = almost full, frame 9 = empty)
        int leakFrame = (int)((1.0f - fuelPercent) * (FUEL_LEAK_FRAMES - 1));
        if (leakFrame < 0) leakFrame = 0;
        if (leakFrame >= FUEL_LEAK_FRAMES) leakFrame = FUEL_LEAK_FRAMES - 1;
        
        hud->currentFuelFrame = leakFrame;
        
    } else {
        // IDLE - animate between idle frames
        hud->fuelIsLeaking = 0;
        
        // Cycle through idle frames every 0.2 seconds
        hud->fuelAnimTimer += 0.016f;  // Assuming ~60 FPS (adjust based on your deltaTime)
        
        if (hud->fuelAnimTimer >= 0.2f) {
            hud->fuelAnimTimer = 0.0f;
            hud->currentFuelFrame = (hud->currentFuelFrame + 1) % FUEL_IDLE_FRAMES;
        }
    }
}

void hud_draw(HUD *hud) {
    if (!hud) return;
    
    // Draw speed meter
    Sprite *speedFrame = hud->speedFrames[hud->currentFrame];
    if (speedFrame) {
        gf2d_sprite_draw(
            speedFrame,
            hud->meterPos,
            &hud->meterScale,
            NULL, // rotation
            NULL, // flip
            NULL, // color
            NULL, // frame
            NULL, // extra
            99    // layer
        );
    }
    
    // Draw fuel gauge
    Sprite *fuelFrame = NULL;
    
    if (hud->fuelIsLeaking) {
        // Use leak animation
        fuelFrame = hud->fuelLeakFrames[hud->currentFuelFrame];
    } else {
        // Use idle animation
        fuelFrame = hud->fuelIdleFrames[hud->currentFuelFrame];
    }
    
    if (fuelFrame) {
        gf2d_sprite_draw(
            fuelFrame,
            hud->fuelPos,
            &hud->fuelScale,
            NULL, // rotation
            NULL, // flip
            NULL, // color
            NULL, // frame
            NULL, // extra
            99    // layer
        );
    }
}

void hud_free(HUD *hud) {
    if (!hud) return;
    // Sprites are managed by sprite system, just free the HUD struct
    free(hud);
}