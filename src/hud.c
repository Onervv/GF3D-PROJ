#include "hud.h"
#include "gf2d_sprite.h"
#include "player.h"
#include "simple_logger.h"

HUD* hud_init() {
    HUD *hud = gfc_allocate_array(sizeof(HUD), 1);
    if (!hud) return NULL;

    // Load speed_1.png → speed_15.png
    for (int i = 0; i < SPEED_FRAMES; i++) {
        char path[256];
        snprintf(path, sizeof(path),
                 "images/ui/speedometer/speed_%d.png", i + 1);

        hud->speedFrames[i] = gf2d_sprite_load_image(path);

        if (!hud->speedFrames[i]) {
            slog("ERROR loading speed frame %d: %s", i, path);
        }
    }

    // Position & scale
    hud->meterPos = gfc_vector2d(50, 560);
    hud->meterScale = gfc_vector2d(0.125f, 0.125f);

    hud->currentFrame = 0;

    return hud;
}

void hud_update(HUD *hud, Entity *player) {
    if (!hud || !player) return;

    PlayerData *pd = (PlayerData*)player->data;
    if (!pd) return;

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
}

void hud_draw(HUD *hud) {
    if (!hud) return;

    Sprite *frame = hud->speedFrames[hud->currentFrame];
    if (!frame) return;

    gf2d_sprite_draw(
        frame,
        hud->meterPos,
        &hud->meterScale,
        NULL, // rotation
        NULL, // flip
        NULL, // color
        NULL, // frame
        NULL, // extra
        99    // layer (high so it appears on top)
    );
}

void hud_free(HUD *hud) {
    if (!hud) return;
    free(hud);
}
