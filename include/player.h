#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"

typedef struct {
	Sint32			maxHealth;
	Sint32			currentHealth;

	Uint32			currentSpeed;
	Uint32			maxSpeed;
	Uint8			onGround; 		//  ground flag
	GFC_Vector3D    groundContact;  // store ground check data

	Uint8 			carIndex; 		// Current car equipped
	Uint8			carIndexMax; 	// Number of cars available

	GFC_List*	    carInventory; 	// List of cars owned by the player
	GFC_Vector3D 	spawnPoint; 	// store initial spawn point for respawn

	// Speed Boost Ability
	Uint8           speedBoostActive;    // Activity flag
    float           speedBoostTimer;     // Time remaining on boost
    float           speedBoostCooldown;  // Cooldown before can boost again
    float           speedBoostMultiplier; // Velocity mult

}PlayerData;

Entity* get_the_player();

Entity* player_spawn(GFC_Vector3D position, GFC_Color color);

void player_think(Entity* self, float deltaTime);

void player_update(Entity* self, float deltaTime);

void player_data_new(PlayerData* data);

#endif