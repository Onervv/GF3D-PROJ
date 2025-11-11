#include "simple_logger.h"
#include "gfc_input.h"

#include "camera_entity.h"
#include "player.h" // Use to track player data

static CameraEntity* ce;  // static (file scope) 

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
    GFC_Vector3D positionOffset = gfc_vector3d(-20, -30, 8);
    ce->target = ce->player->position; // Always look at player

    // --- Vertical adjustment (zoom in/out or tilt up/down) ---
    if (gfc_input_command_down("panup")) ce->zOffset += 0.3f;
    if (gfc_input_command_down("pandown")) ce->zOffset -= 0.3f;
 	ce->zOffset = clamp(ce->zOffset, -2, 14);

    // --- Horizontal "lean" effect when panning ---
    static float cameraLean = 0.0f;
    const float leanAmount = 5.0f;     // how far the camera can lean left/right
    const float leanSpeed = 0.35f;     // how fast the camera leans

    // Smooth lean target based on player input
    if (gfc_input_command_down("panleft")) {
        cameraLean += (leanAmount - cameraLean) * leanSpeed;
    } else if (gfc_input_command_down("panright")) {
        cameraLean += (-leanAmount - cameraLean) * leanSpeed;
    } else {
        // Ease back to center when no pan input
        cameraLean += (0.0f - cameraLean) * leanSpeed;
    }

    // Compute sideways vector (perpendicular to forward)
    GFC_Vector2D rightDir = { -direction2d.y, direction2d.x };

    // Apply lean offset to camera position
    ce->position.x = ce->player->position.x + (direction2d.x * positionOffset.x) + (rightDir.x * cameraLean);
    ce->position.y = ce->player->position.y + (direction2d.y * positionOffset.y) + (rightDir.y * cameraLean);
    ce->position.z = ce->player->position.z + positionOffset.z;

    // Raise target by z offset
    ce->target.z += ce->zOffset;

    // --- Apply final look ---
    gf3d_camera_look_at(ce->target, &ce->position);
}
