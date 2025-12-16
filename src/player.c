#include "physics.h"
#include "player.h"
#include "simple_logger.h"
#include "gfc_input.h"
#include "world.h"
#include "gf3d_particle.h"


// Constants
// #define PLAYER_RADIUS     	    1.0f;
// #define DOWN_RAY_ABOVE       	1.0f
// #define DOWN_RAY_BELOW       	2.0f
#define PENETRATION_TOLERANCE 	0.05f
#define REST_TOLERANCE       	0.02f

#define JUMP_STRENGTH        	1.1f
#define MOVE_SPEED           	0.5f

#define SPEED_BOOST_DURATION    2.0f   // Boost lasts 2 seconds
#define SPEED_BOOST_COOLDOWN    5.0f   // 5 second cooldown
#define SPEED_BOOST_MULTIPLIER  2.0f   // 2x speed

static Entity* thePlayer;

// --- Static accessor ---
Entity* get_the_player() {
    if (!thePlayer) {
        slog("No static player to return");
        return NULL;
    }
    return thePlayer;
}

// --- Initialize PlayerData ---
void player_data_new(PlayerData* data) {
    if (!data) return;
    data->maxHealth = 100;
    data->currentHealth = 100;
    data->currentSpeed = 0;
    data->maxSpeed = 4;
    data->onGround = 0;
    data->carInventory = gfc_list_new();
    data->carIndex = 0;
    data->carIndexMax = 0;

     // Speed boost initialization
    data->speedBoostActive = 0;
    data->speedBoostTimer = 0.0f;
    data->speedBoostCooldown = 0.0f;
    data->speedBoostMultiplier = 2.0f;  // 2x for now
}

Entity* player_spawn(GFC_Vector3D position, GFC_Color color) {
    Entity* self;
    PlayerData* data;

    self = entity_new();
    if (!self) return NULL;

    data = gfc_allocate_array(sizeof(PlayerData), 1);
    player_data_new(data);

    data->onGround = 0; // start in air
    data->spawnPoint = position; // THIS CAUSES SEG FAULT LAST TIME WATCH IT
    self->data = data;

    gfc_line_cpy(self->name, "Player");
    self->mesh = gf3d_mesh_load("models/primitives/bigSphere.obj");
    self->texture = gf3d_texture_load("models/primitives/flatwhite.png");
    self->color = color;
    self->position = position;
    self->rotation = gfc_vector3d(0, 0, -2 * GFC_PI);
    self->scale = gfc_vector3d(1, 1, 1);

    self->drawShadow = 1;  //Enable shadows

    self->think = player_think;
    self->update = player_update;

    thePlayer = self;
    return self;
}

// --- Player movement logic ---
void player_think(Entity* self, float deltaTime) {
    if (!self) return;
    PlayerData* pdata = (PlayerData*)self->data;
    if (!pdata) return;
    
    GFC_Vector2D direction2d;
    float move = 0;
    float moveStep = 0.09f;
    const float turnStep = 0.03f;
    
    //////////////////////////////////
    //========= SPEED BOOST ========/
    /////////////////////////////////

    // Update boost timer
    if (pdata->speedBoostTimer > 0) {
        pdata->speedBoostTimer -= deltaTime;
        if (pdata->speedBoostTimer <= 0) {
            pdata->speedBoostActive = 0;
            pdata->speedBoostTimer = 0;
            slog("Speed boost ended");
        }
    }
    
    // Update cooldown timer
    if (pdata->speedBoostCooldown > 0) {
        pdata->speedBoostCooldown -= deltaTime;
        if (pdata->speedBoostCooldown < 0) {
            pdata->speedBoostCooldown = 0;
        }
    }
    
    // Activate speed boost
    if (gfc_input_command_down("boost") && 
        pdata->speedBoostCooldown <= 0 && 
        !pdata->speedBoostActive) {
        pdata->speedBoostActive = 1;
        pdata->speedBoostTimer = SPEED_BOOST_DURATION;
        pdata->speedBoostCooldown = SPEED_BOOST_COOLDOWN;
        slog("Speed boost activated!");
    }
    ///////////////////////////////////
    // ========== MOVEMENT ==========//
    ///////////////////////////////////

    if (pdata->speedBoostActive) {
        moveStep *= SPEED_BOOST_MULTIPLIER;
        self->color = gfc_color(1.0f, 0.5f, 0.0f, 1.0f);  // Orange
        
        // Spawn trail particles while boosting
        if (rand() % 3 == 0) {  // 33% chance each frame
            gf3d_particle_spawn(
                self->position,
                gfc_vector3d(
                    (rand() % 40 - 20) / 20.0f,  // Small random spread
                    (rand() % 40 - 20) / 20.0f,
                    0.5f + (rand() % 30) / 100.0f  // Slight upward: 0.5-0.8
                ),
                gfc_color(1.0f, 0.5f, 0.0f, 1.0f),  // Orange
                0.8f,  // duration of fade
                1.0f   // size
            );
        }
    } else {
        self->color = GFC_COLOR_WHITE;
    }

    // Rotate left/right
    if (gfc_input_command_down("panleft"))  self->rotation.z += turnStep;
    if (gfc_input_command_down("panright")) self->rotation.z -= turnStep;
    
    // Forward/back movement
    direction2d = gfc_vector2d_from_angle(self->rotation.z);
    gfc_vector2d_normalize(&direction2d);
    
    if (gfc_input_command_down("moveforward")) move += moveStep;
    if (gfc_input_command_down("moveback"))    move -= moveStep;
    
    if (move != 0) {
        GFC_Vector3D force = { direction2d.x * move, direction2d.y * move, 0 };
        physics_add_force(self, force);
    }
    
    // Jump
    if (gfc_input_command_down("jump") && pdata && pdata->onGround) {
        self->velocity.z = JUMP_STRENGTH;
        pdata->onGround = 0;
    }
    
    // Apply physics
    physics_apply(self, pdata->onGround, 0);
}


void player_update(Entity* self, float deltaTime) {
    if (!self) return;
    PlayerData* pdata = (PlayerData*)self->data;
    if (!pdata) return;
	// slog("Player position: x=%.2f, y=%.2f, z=%.2f", self->position.x, self->position.y, self->position.z);
    // Contained game world magic here (ligit just set player location back to origin)
    if (self->position.z < -100.0f) { // threshold
		slog("here");
        self->position = pdata->spawnPoint;
        self->velocity = gfc_vector3d(0, 0, 0); // reset momentum
        pdata->onGround = 0; // reset grounded state
    }

    entity_move(self); // move the entity player
    
}
