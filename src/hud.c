#include "hud.h"
#include "gf2d_sprite.h"
#include "player.h"
#include "simple_logger.h"
#include "lap_manager.h"
#include "gf2d_font.h"

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
    
    // Fuel gauge position & scale
    hud->fuelPos = gfc_vector2d(0, 0);
    hud->fuelScale = gfc_vector2d(1.0, 1.0f);
    hud->currentFuelFrame = 0;
    hud->fuelIsLeaking = 0;
    hud->fuelAnimTimer = 0.0f;
    
    // Lap counter position (top right) (NEW)
    hud->lapTextPos = gfc_vector2d(1100, 10);  // Adjust based on your screen resolution
    strcpy(hud->lapText, "Lap: 1/3");
    strcpy(hud->pickupText, "Pickups: 0/8");
    
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
    
    // ===== UPDATE FUEL GAUGE (existing code) =====
    if (pd->speedBoostActive || pd->speedBoostCooldown > 0) {
        hud->fuelIsLeaking = 1;
        
        float fuelPercent;
        if (pd->speedBoostActive) {
            fuelPercent = pd->speedBoostTimer / 2.0f;
        } else {
            fuelPercent = 1.0f - (pd->speedBoostCooldown / 5.0f);
        }
        
        if (fuelPercent < 0.0f) fuelPercent = 0.0f;
        if (fuelPercent > 1.0f) fuelPercent = 1.0f;
        
        int leakFrame = (int)((1.0f - fuelPercent) * (FUEL_LEAK_FRAMES - 1));
        if (leakFrame < 0) leakFrame = 0;
        if (leakFrame >= FUEL_LEAK_FRAMES) leakFrame = FUEL_LEAK_FRAMES - 1;
        
        hud->currentFuelFrame = leakFrame;
        
    } else {
        hud->fuelIsLeaking = 0;
        hud->fuelAnimTimer += 0.016f;
        
        if (hud->fuelAnimTimer >= 0.2f) {
            hud->fuelAnimTimer = 0.0f;
            hud->currentFuelFrame = (hud->currentFuelFrame + 1) % FUEL_IDLE_FRAMES;
        }
    }
    
    // ===== UPDATE LAP TEXT (NEW) =====
    LapManager* lapMgr = lap_manager_get();
    if (lapMgr) {
        snprintf(hud->lapText, sizeof(hud->lapText), 
                 "Lap: %d/%d", lapMgr->currentLap, TOTAL_LAPS);
        snprintf(hud->pickupText, sizeof(hud->pickupText), 
                 "Pickups: %d/%d", lapMgr->pickupsCollected, lapMgr->totalPickups);
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
            NULL, NULL, NULL, NULL, NULL, 99
        );
    }
    
    // Draw fuel gauge
    Sprite *fuelFrame = NULL;
    if (hud->fuelIsLeaking) {
        fuelFrame = hud->fuelLeakFrames[hud->currentFuelFrame];
    } else {
        fuelFrame = hud->fuelIdleFrames[hud->currentFuelFrame];
    }
    
    if (fuelFrame) {
        gf2d_sprite_draw(
            fuelFrame,
            hud->fuelPos,
            &hud->fuelScale,
            NULL, NULL, NULL, NULL, NULL, 99
        );
    }
    
    // Draw lap counter text
    gf2d_font_draw_line_tag(hud->lapText, FT_H1, GFC_COLOR_WHITE, hud->lapTextPos);
    
    GFC_Vector2D pickupPos = hud->lapTextPos;
    pickupPos.y += 30;  // Draw pickup text below lap text
    gf2d_font_draw_line_tag(hud->pickupText, FT_H1, GFC_COLOR_WHITE, pickupPos);
}

void hud_free(HUD *hud) {
    if (!hud) return;
    // Sprites are managed by sprite system, just free the HUD struct
    free(hud);
}