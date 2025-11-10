#include "simple_logger.h"
#include "gfc_input.h"

#include "player.h"
#include "camera_entity.h"

static Entity* thePlayer;

Entity* get_the_player() {
	if (!thePlayer) {
		slog("No static player to return");
		return NULL;
	}
	return thePlayer;
}

Entity* player_spawn(GFC_Vector3D position, GFC_Color color) {
	Entity* self;
	PlayerData* data;

	self = entity_new();
	if (!self)return;

	data = gfc_allocate_array(sizeof(PlayerData), 1);
	gfc_line_cpy(self->name, "Player");
	self->mesh = gf3d_mesh_load("models/primitives/isphere.obj");
	self->texture = gf3d_texture_load("models/primitives/flatwhite.png");
	self->color = color;
	self->position = position;
	self->rotation = gfc_vector3d(0, 0, -2 * GFC_PI);
	//entity defaults to scale of 1, 1, 1
	self->think = player_think;
	self->update = player_update;

	player_data_new(data);
	self->data = data;

	thePlayer = self; //assign static variable
	return self;
}

void player_think(Entity* self) {
	Uint32 mouseState;
	PlayerData* data;

	int mx, my;
	GFC_Vector2D direction2d;
	float move = 0; 
	float moveStep = 1.5;


	if (!self) return;
	//data = self->data;
	//if (!data) return;

	self->velocity.x = 0;
	self->velocity.y = 0;
	self->velocity.z = 0;

	if (gfc_input_command_down("panleft")) {
		self->rotation.z += .1;
	}
	if (gfc_input_command_down("panright")) {
		self->rotation.z -= .1;
	}

	direction2d = gfc_vector2d_from_angle(self->rotation.z);
	gfc_vector2d_normalize(&direction2d);
	if (gfc_input_command_down("moveforward")) {
		move += moveStep;
	}
	if (gfc_input_command_down("moveback")) {
		move -= moveStep;
	}
	if (move) {
		gfc_vector2d_scale(direction2d, direction2d, move);
		gfc_vector2d_add(self->velocity, self->velocity, direction2d);
	}
	move = 0;
	direction2d = gfc_vector2d_from_angle(self->rotation.z);
	gfc_vector2d_normalize(&direction2d);
	direction2d = gfc_vector2d_rotate(direction2d, GFC_HALF_PI);
	if (gfc_input_command_down("moveright")) {
		move -= moveStep;
	}
	if (gfc_input_command_down("moveleft")) {
		move += moveStep;
	}
	if (move) {
		gfc_vector2d_scale(direction2d, direction2d, move);
		gfc_vector2d_add(self->velocity, self->velocity, direction2d);
	}
	if (gfc_input_command_down("jump")) {
		self->velocity.z += 1;
	}
	if (gfc_input_command_down("crouch")) {
		self->velocity.z -= 1;
	}

	mouseState = SDL_GetMouseState(&mx, &my);
}

void player_update(Entity* self) {
	PlayerData* data;

	if (!self) return;
	//data = self->data;
	//if (!data) return;

	entity_move(self); 

	self->bounds.x = self->position.x;
	self->bounds.y = self->position.y;
	self->bounds.z = self->position.z;
}

void player_data_new(PlayerData* data) {
	data->carInventory = gfc_list_new();
}