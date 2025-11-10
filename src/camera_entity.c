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

void camera_think(CameraEntity* ce) {
	GFC_Vector3D playerRotation = ce->player->rotation;
	GFC_Vector2D direction2d;
	direction2d = gfc_vector2d_from_angle(playerRotation.z);
	gfc_vector2d_normalize(&direction2d);

	GFC_Vector3D positionOffset = gfc_vector3d(-20, -20, 12); // offset from player, stays right on him
	ce->target = ce->player->position; // look at player position
	

	if (gfc_input_command_down("panup")) {
		ce->zOffset += .3;
	}
	if (gfc_input_command_down("pandown")) {
		ce->zOffset -= .3;
	}

	if (ce->zOffset > 14) {
		ce->zOffset = 14;
	}
	if (ce->zOffset < -2) {
		ce->zOffset = -2;
	}

	ce->position.x = ce->player->position.x + (direction2d.x * positionOffset.x);
	ce->position.y = ce->player->position.y + (direction2d.y * positionOffset.y);
	ce->position.z = ce->player->position.z + positionOffset.z;

	ce->target.z += ce->zOffset;

	gf3d_camera_look_at(ce->target, &ce->position);
}