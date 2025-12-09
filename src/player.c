#include "player.h"
#include "simple_logger.h"
#include "gfc_input.h"
#include "world.h"

// Constants
#define PLAYER_RADIUS        	1.0f
#define DOWN_RAY_ABOVE       	1.0f
#define DOWN_RAY_BELOW       	2.0f
#define PENETRATION_TOLERANCE 	0.05f
#define REST_TOLERANCE       	0.02f

#define JUMP_STRENGTH        	1.1f
#define MOVE_SPEED           	0.5f

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

    self->think = player_think;
    self->update = player_update;

    thePlayer = self;
    return self;
}

// --- Player movement logic ---
void player_think(Entity* self) {
    if (!self) return;
    PlayerData* pdata = (PlayerData*)self->data;
    if (!pdata) return;

    GFC_Vector2D direction2d;
    float move = 0;
    const float moveStep = 0.09f;
    const float turnStep = 0.03f;

    Uint8 jumpThisFrame = 0;

    // Rotate left/right
    if (gfc_input_command_down("panleft"))  self->rotation.z += turnStep;
    if (gfc_input_command_down("panright")) self->rotation.z -= turnStep;

    // Forward/back
    direction2d = gfc_vector2d_from_angle(self->rotation.z);
    gfc_vector2d_normalize(&direction2d);
    if (gfc_input_command_down("moveforward")) move += moveStep;
    if (gfc_input_command_down("moveback"))    move -= moveStep;

    if (move != 0) {
        GFC_Vector3D force = { direction2d.x * move, direction2d.y * move, 0 };
        physics_add_force(self, force);
    }

   if (gfc_input_command_down("jump") && pdata && pdata->onGround) {
    self->velocity.z = JUMP_STRENGTH;
    pdata->onGround = 0; // immediately leave ground
}


    // Physics handling (ignore gravity if jump just happened)
    physics_apply(self, pdata->onGround, jumpThisFrame);

    // Move entity
    entity_move(self);
}


void player_update(Entity* self) {
    if (!self) return;
    PlayerData* pdata = (PlayerData*)self->data;
    if (!pdata) return;
	// slog("Player position: x=%.2f, y=%.2f, z=%.2f", self->position.x, self->position.y, self->position.z);
    // --- Respawn if fallen too far ---
    if (self->position.z < -100.0f) { // threshold
		slog("here");
        self->position = pdata->spawnPoint;
        self->velocity = gfc_vector3d(0, 0, 0); // reset momentum
        pdata->onGround = 0; // reset grounded state
    }

    // --- Normal update / think ---
    if (self->think) self->think(self);
}
