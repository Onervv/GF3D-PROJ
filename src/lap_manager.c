#include "lap_manager.h"
#include "pickup.h"
#include "world.h"
#include "simple_logger.h"
#include <stdlib.h>

static LapManager gLapManager = {0};

void lap_manager_init() {
    memset(&gLapManager, 0, sizeof(LapManager));
    
    gLapManager.currentLap = 1;
    gLapManager.pickupsCollected = 0;
    gLapManager.totalPickups = 0;
    gLapManager.activePickups = gfc_list_new();
    gLapManager.lapComplete = 0;
    gLapManager.raceComplete = 0;
    strcpy(gLapManager.nextWorldFile, "defs/terrain/terrain2.def");
    
    slog("=== Lap Manager initialized ===");
    slog("Current lap: %d", gLapManager.currentLap);
}

LapManager* lap_manager_get() {
    return &gLapManager;
}

void lap_manager_spawn_pickups(World* world, int count) {

    slog("=== lap_manager_spawn_pickups called ===");
    slog("World pointer: %p", world);
    slog("World mesh pointer: %p", world ? world->mesh : NULL);
    slog("Requested pickups: %d", count);
    
    if (!world || !world->mesh) {
        slog("ERROR: Cannot spawn pickups - invalid world");
        return;
    }
    
    // Clear any existing pickups
    if (gLapManager.activePickups) {
        gfc_list_clear(gLapManager.activePickups);
    }
    
    gLapManager.totalPickups = count;
    gLapManager.pickupsCollected = 0;
    
    // Get world bounds to generate random positions
    const float spawnRadius = 1000.0f;  // Radius around origin to spawn pickups
    const float spawnHeight = 100.0f;  // Height to start raycasts from
    const float minSpacing = 30.0f;    // Minimum distance between pickups
    
    GFC_List* spawnPositions = gfc_list_new();
    int attempts = 0;
    int maxAttempts = count * 20;
    
    while (gfc_list_get_count(spawnPositions) < count && attempts < maxAttempts) {
        attempts++;
        
        // Generate random position in circle
        float angle = ((float)(rand() % 360)) * (GFC_PI / 180.0f);
        float distance = ((float)(rand() % 100) / 100.0f) * spawnRadius;
        
        GFC_Vector3D testPos = gfc_vector3d(
            cos(angle) * distance,
            sin(angle) * distance,
            spawnHeight
        );
        
        // Raycast down to find ground
        GFC_Vector3D groundPos = testPos;
        groundPos.z = -10.0f;
        
        GFC_Vector3D contact;
        if (world_edge_test(world, testPos, groundPos, &contact)) {
            // Found ground contact
            GFC_Vector3D* spawnPos = gfc_allocate_array(sizeof(GFC_Vector3D), 1);
            *spawnPos = contact;
            spawnPos->z += 0.5f;
            
            // Check spacing from other pickups
            Uint8 tooClose = 0;
            int spawnCount = gfc_list_get_count(spawnPositions);
            for (int i = 0; i < spawnCount; i++) {
                GFC_Vector3D* existingPos = (GFC_Vector3D*)gfc_list_get_nth(spawnPositions, i);
                if (existingPos) {
                    float dx = spawnPos->x - existingPos->x;
                    float dy = spawnPos->y - existingPos->y;
                    float dist = sqrt(dx * dx + dy * dy);
                    
                    if (dist < minSpacing) {
                        tooClose = 1;
                        break;
                    }
                }
            }  // CLOSE for loop
            
            if (!tooClose) {
                gfc_list_append(spawnPositions, spawnPos);
            } else {
                free(spawnPos);
            }
        }  // CLOSE if (world_edge_test)
    }  // CLOSE while loop
    
    // Spawn pickup entities at the valid positions
    int spawnCount = gfc_list_get_count(spawnPositions);
    slog("Spawning %d pickups (requested %d)", spawnCount, count);
    
    for (int i = 0; i < spawnCount; i++) {
        GFC_Vector3D* pos = (GFC_Vector3D*)gfc_list_get_nth(spawnPositions, i);
        if (pos) {
            Entity* pickup = pickup_spawn(*pos, i);
            if (pickup) {
                gfc_list_append(gLapManager.activePickups, pickup);
            }
            free(pos);
        }
    }  // CLOSE for loop
    
    gfc_list_delete(spawnPositions);
    gLapManager.totalPickups = spawnCount;
    
    slog("Lap %d started with %d pickups", gLapManager.currentLap, gLapManager.totalPickups);
}  

void lap_manager_pickup_collected() {
    gLapManager.pickupsCollected++;
    slog("Pickup collected! (%d/%d)", gLapManager.pickupsCollected, gLapManager.totalPickups);
    
    // Check if lap is complete
    if (gLapManager.pickupsCollected >= gLapManager.totalPickups) {
        gLapManager.lapComplete = 1;
        slog("Lap %d complete!", gLapManager.currentLap);
    }
}

void lap_manager_update() {
    // Check if lap is complete and should progress
    if (gLapManager.lapComplete && !gLapManager.raceComplete) {
        if (gLapManager.currentLap >= TOTAL_LAPS) {
            gLapManager.raceComplete = 1;
            slog("Race complete! All %d laps finished!", TOTAL_LAPS);
            // TODO: You could trigger victory screen here
        }
    }
}

void lap_manager_next_lap() {
    if (gLapManager.raceComplete) {
        slog("Race already complete!");
        return;
    }
    
    gLapManager.currentLap++;
    gLapManager.lapComplete = 0;
    
    // Update next world file name
    snprintf(gLapManager.nextWorldFile, sizeof(gLapManager.nextWorldFile), 
             "defs/terrain/terrain%d.def", gLapManager.currentLap);
    
    slog("Starting lap %d", gLapManager.currentLap);
}

void lap_manager_close() {
    if (gLapManager.activePickups) {
        gfc_list_delete(gLapManager.activePickups);
        gLapManager.activePickups = NULL;
    }
    memset(&gLapManager, 0, sizeof(LapManager));
    slog("Lap Manager closed");
}