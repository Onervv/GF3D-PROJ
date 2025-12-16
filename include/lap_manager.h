#ifndef __LAP_MANAGER_H__
#define __LAP_MANAGER_H__

#include "gfc_vector.h"
#include "gfc_list.h"
#include "world.h"

#define MAX_PICKUPS_PER_LAP 8
#define TOTAL_LAPS 3

typedef struct {
    int currentLap;             // Current lap (1-3)
    int pickupsCollected;       // Pickups collected this lap
    int totalPickups;           // Total pickups spawned this lap
    GFC_List* activePickups;    // List of pickup entities
    Uint8 lapComplete;          // Flag when lap is done
    Uint8 raceComplete;         // Flag when all laps done
    char nextWorldFile[256];    // Next world to load
} LapManager;

/**
 * @brief Initialize the lap manager
 */
void lap_manager_init();

/**
 * @brief Get the global lap manager
 */
LapManager* lap_manager_get();

/**
 * @brief Spawn pickups on the current world
 * @param world The world to spawn pickups in
 * @param count Number of pickups to spawn
 */
void lap_manager_spawn_pickups(World* world, int count);

/**
 * @brief Notify that a pickup was collected
 */
void lap_manager_pickup_collected();

/**
 * @brief Update lap manager (check for lap completion)
 */
void lap_manager_update();

/**
 * @brief Start next lap
 */
void lap_manager_next_lap();

/**
 * @brief Clean up lap manager
 */
void lap_manager_close();

#endif