#include "pickup.h"
#include "player.h"
#include "gf3d_mesh.h"
#include "simple_logger.h"
#include "gf3d_mesh.h"
#include "lap_manager.h"  
#include <math.h>

void pickup_think(Entity* self, float deltaTime);
void pickup_update(Entity* self, float deltaTime);
void pickup_free(Entity* self);

Entity* pickup_spawn(GFC_Vector3D position, int pickupID) {
    Entity* self = entity_new();
    if (!self) return NULL;
    
    PickupData* data = gfc_allocate_array(sizeof(PickupData), 1);
    if (!data) {
        entity_free(self);
        return NULL;
    }
    
    data->pickupID = pickupID;
    data->collected = 0;
    data->spinSpeed = 2.0f;
    
    self->data = data;
    self->type = ENTITY_TYPE_PICKUP;  // Make sure this line exists
    snprintf(self->name, sizeof(self->name), "Pickup_%d", pickupID);
    
    self->mesh = gf3d_mesh_load("models/primitives/cube.obj");
    self->texture = gf3d_texture_load("models/primitives/flatwhite.png");
    self->color = gfc_color(1.0f, 0.8f, 0.0f, 1.0f);
    
    self->position = position;
    self->position.z += 2.0f;
    self->rotation = gfc_vector3d(0, 0, 0);
    self->scale = gfc_vector3d(2.0f, 2.0f, 2.0f);
    
    self->drawShadow = 0;
    
    self->think = pickup_think;
    self->update = pickup_update;
    self->free = pickup_free;  

    slog("Spawned pickup %d at (%.2f, %.2f, %.2f)", 
         pickupID, position.x, position.y, position.z);
    
    return self;
}

void pickup_draw(Entity* self) {
    if (!self) return;
    
    PickupData* data = (PickupData*)self->data;
    
    // FIX 2: Removed "|| data->collected" from this check.
    // If we return here, we can't see the cool fade-out effect!
    if (!data) return; 
    
    entity_move(self);
    
    if (self->mesh && self->texture) {
        gf3d_mesh_draw(
            self->mesh, 
            self->matrix, 
            self->color, 
            self->texture, 
            gfc_vector3d(0, 0, 25), 
            GFC_COLOR_WHITE
        );
    }
}

void pickup_think(Entity* self, float deltaTime) {
    if (!self) return;
    
    PickupData* data = (PickupData*)self->data;
    if (!data || data->collected) return;
    
    // Spin the pickup
    self->rotation.z += data->spinSpeed * deltaTime;
    
    // Bob up and down
    static float bobTimer = 0.0f;
    bobTimer += deltaTime * 2.0f;
    self->position.z += sin(bobTimer) * 0.01f;
    
    // Check for player collection
    Entity* player = get_the_player();
    if (player) {
        pickup_check_collection(self, player);
    }
}

void pickup_update(Entity* self, float deltaTime) {
    if (!self) return;
    
    PickupData* data = (PickupData*)self->data;
    if (!data) return;
    
    // If collected, fade out and remove
    if (data->collected) {
        self->color.a -= deltaTime * 3.0f;  // Fade out
        
        if (self->color.a <= 0.0f) {
            // FIX 1: Save the rest of the pickups!
            // We set the mesh to NULL so entity_free doesn't destroy 
            // the shared model resource that other pickups are using.
            self->mesh = NULL; 
            
            entity_free(self);
        }
    }
}

void pickup_free(Entity* self) {
    if (!self) return;
    if (self->data) {
        free(self->data);
        self->data = NULL;
    }
}

Uint8 pickup_check_collection(Entity* pickup, Entity* player) {
    if (!pickup || !player) return 0;
    
    PickupData* data = (PickupData*)pickup->data;
    if (!data || data->collected) return 0;
    
    GFC_Vector3D diff = gfc_vector3d(
        player->position.x - pickup->position.x,
        player->position.y - pickup->position.y,
        player->position.z - pickup->position.z
    );
    
    float distance = gfc_vector3d_magnitude(diff);
    
    if (distance < PICKUP_RADIUS) {
        data->collected = 1;
        slog("Pickup %d collected!", data->pickupID);
        
       
        lap_manager_pickup_collected();
        
        return 1;
    }
    
    return 0;
}
