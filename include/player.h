#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef struct {
	Sint32			maxHealth;
	Sint32			currentHealth;

	Uint32			currentSpeed;
	Uint32			maxSpeed
}PlayerData;

Entity* get_the_player();

Entity* player_spawn(GFC_Vector3D position, GFC_Color color);

void player_think(Entity* self);

void player_update(Entity* self);

void player_data_new(PlayerData* data);

#endif