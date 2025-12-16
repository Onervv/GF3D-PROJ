#include "simple_logger.h"
#include "gfc_input.h"

#include "camera_entity.h"
#include "player.h" // Use to track player data

static CameraEntity* ce;  

CameraEntity* camera_entity_new() {
	ce = gfc_allocate_array(sizeof(CameraEntity), 1);
	ce->player = get_the_player(); // Link to player entity
	if (!ce->player){
		slog("camera_entity_new: Failed to get player entity");
	}
	ce->position = gfc_vector3d(0, 0, 0); // initialize position for now
	ce->target = gfc_vector3d(0, 0, 0);   // initialize target for now
	ce->zOffset = 12; // default vertical offset
	return ce;
}

// Clamp for camera zoffset
static inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void camera_think(CameraEntity* ce) {
    if (!ce || !ce->player) return;

    GFC_Vector3D playerRotation = ce->player->rotation;
    GFC_Vector2D direction2d = gfc_vector2d_from_angle(playerRotation.z);
    gfc_vector2d_normalize(&direction2d);

    // --- Base camera offset ---
    // Moved Z to 8 for a better default view
    GFC_Vector3D positionOffset = gfc_vector3d(-20, -30, 8); 
    ce->target = ce->player->position;

    // --- Vertical adjustment ---
    if (gfc_input_command_down("panup")) ce->zOffset += 0.3f;
    if (gfc_input_command_down("pandown")) ce->zOffset -= 0.3f;
    ce->zOffset = clamp(ce->zOffset, -2, 14);

    // --- SMOOTH HORIZONTAL LEAN ---
    static float cameraLean = 0.0f;
    const float leanAmount = 8.0f;     // Increased range slightly for better visibility
    const float leanSmooth = 0.15f;    // Lower = smoother easing at the end
    const float maxSpeed = 3.0f;       // The "Speed Limit" per frame

    
    float targetLean = 0.0f;
    if (gfc_input_command_down("panleft")) {
        targetLean = leanAmount;
    } else if (gfc_input_command_down("panright")) {
        targetLean = -leanAmount;
    }

    
    float diff = targetLean - cameraLean;

    float step = diff * leanSmooth;

    
    if (step > maxSpeed) step = maxSpeed;
    if (step < -maxSpeed) step = -maxSpeed;

    
    cameraLean += step;

    GFC_Vector2D rightDir = { -direction2d.y, direction2d.x };

    ce->position.x = ce->player->position.x + (direction2d.x * positionOffset.x) + (rightDir.x * cameraLean);
    ce->position.y = ce->player->position.y + (direction2d.y * positionOffset.y) + (rightDir.y * cameraLean);
    ce->position.z = ce->player->position.z + positionOffset.z;

    ce->target.z += ce->zOffset;

    gf3d_camera_look_at(ce->target, &ce->position);
}