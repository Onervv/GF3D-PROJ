#ifndef __PICKUP_H__
#define __PICKUP_H__

#include "entity.h"
#include "gfc_vector.h"

typedef struct {
    int pickupID;       // Unique ID for this pickup
    Uint8 collected;    // Has this been collected?
    float spinSpeed;    // Rotation speed for visual effect
} PickupData;

/**
 * @brief Spawn a pickup at a position
 * @param position Where to spawn the pickup
 * @param pickupID Unique ID for this pickup
 * @return The spawned pickup entity
 */
Entity* pickup_spawn(GFC_Vector3D position, int pickupID);

/**
 * @brief Check if player is near pickup and collect it
 * @param pickup The pickup entity
 * @param player The player entity
 * @return 1 if collected, 0 otherwise
 */
Uint8 pickup_check_collection(Entity* pickup, Entity* player);

#endif