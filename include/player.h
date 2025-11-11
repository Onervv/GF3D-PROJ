#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef struct {
	Sint32			maxHealth;
	Sint32			currentHealth;

	Uint32			currentSpeed;
	Uint32			maxSpeed;
	Uint8			onGround; // 1 if player on ground 0 if in air.

	Uint8 			carIndex; 		// Current car equipped
	Uint8			carIndexMax; 	// Number of cars available

	GFC_List*	    carInventory; 	// List of cars owned by the player

}PlayerData;

Entity* get_the_player();

Entity* player_spawn(GFC_Vector3D position, GFC_Color color);

void player_think(Entity* self);

void player_update(Entity* self);

void player_data_new(PlayerData* data);

#endif