#include "simple_logger.h"
#include "gfc_input.h"

#include "player.h"

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
	self->mesh = gf3d_mesh_load("models/dino/dino.obj");
	self->texture = gf3d_texture_load("models/dino/dino.png");
	self->color = color;
	self->position = position;
	self->rotation = gfc_vector3d(0, 0, 0);
	//entity defaults to scale of 1, 1, 1
	self->think = player_think;
	self->update = player_update;

	player_data_new(data);

	thePlayer = self; //assign static variable
	return self;
}

void player_think(Entity* self) {
	Uint32 mouseState;
	PlayerData* data;
	int mx, my;

	if (!self) return;
	//data = self->data;
	//if (!data) return;

	self->velocity.x = 0;
	self->velocity.y = 0;
	self->velocity.z = 0;

	if (gfc_input_command_down("moveforward")) {
		self->velocity.y += 1;
	}
	if (gfc_input_command_down("moveback")) {
		self->velocity.y -= 1;
	}
	if (gfc_input_command_down("moveright")) {
		self->velocity.x += 1;
	}
	if (gfc_input_command_down("moveleft")) {
		self->velocity.x -= 1;
	}
	if (gfc_input_command_down("jump")) {
		self->velocity.z += 1;
	}

	// Normalize now but scale later, for car movement we need magnitude to simulate amount of gas/brake
	// dont forget to cap speed too
	gfc_vector3d_normalize(&self->velocity);

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
	data = gfc_allocate_array(sizeof(PlayerData), 1);
}