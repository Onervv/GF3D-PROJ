#include "simple_logger.h"

#include "entity.h"
#include "world.h"
#include "player.h"

typedef struct {
	Entity* entity_list;
	Uint32 entity_max;
}EntitySystem;

static EntitySystem entity_system = { 0 };

Entity* entity_new() {
	int i;
	if (entity_system.entity_list) {
		for (i = 0; i < entity_system.entity_max; i++) {
			if (!entity_system.entity_list[i]._inuse) {
				entity_system.entity_list[i]._inuse = 1;
				//set defaults like color and texture
				entity_system.entity_list[i].color = GFC_COLOR_WHITE;
				entity_system.entity_list[i].scale = gfc_vector3d(1, 1, 1);
				return &entity_system.entity_list[i];
			}
		}
	}
	return NULL;
}

void entity_free(Entity* e) {
	if (!e) return;
	gf3d_mesh_free(e->mesh);
	gf3d_texture_free(e->texture);
	memset(e, 0, sizeof(Entity));
}

void entity_system_init(Uint16 maxEnts) {
	if (!maxEnts) {
		slog("Invalid maxEnts for entity system init");
		return;
	}
	entity_system.entity_list = gfc_allocate_array(sizeof(Entity), maxEnts);
	if (!entity_system.entity_list) {
		slog("Failed to allocate entities in entity system");
		return;
	}
	entity_system.entity_max = maxEnts;
	slog("Entity system initialized with %i entities", entity_system.entity_max);
	atexit(entity_system_close);
}

void entity_system_close() {
	int i;
	if (entity_system.entity_list) {
		for (i = 0; i < entity_system.entity_max; i++) {
			if (entity_system.entity_list[i]._inuse) {
				entity_free(&entity_system.entity_list[i]);
			}
		}
	}
}

void entity_move(Entity* self) {
    if (!self) return;
    PlayerData* pdata = (PlayerData*)self->data;

    World* world = get_the_world();
    if (!world) return;

    // --- Predict next position ---
    GFC_Vector3D predicted;
    gfc_vector3d_add(predicted, self->position, self->velocity);

    // --- Horizontal movement ---
	self->position.x += self->velocity.x/1.35;
	self->position.y += self->velocity.y/1.35;
	

    // --- Ground detection ---
    GFC_Vector3D start = self->position;
    start.z += PLAYER_RADIUS;
    GFC_Vector3D end = start;
    end.z -= DOWN_RAY_BELOW;
    GFC_Vector3D contact;

    float groundZ = -9999.0f;
    int hit = 0;
    if (world_edge_test(world, start, end, &contact)) {
        groundZ = contact.z + PLAYER_RADIUS;
        hit = 1;
    }

    const float GROUND_BUFFER = 0.1f; // snap threshold

    if (hit && self->velocity.z <= 0 && predicted.z <= groundZ + GROUND_BUFFER) {
        // Snap to ground if falling and close
        self->position.z = groundZ;
        self->velocity.z = 0;
        if (pdata) pdata->onGround = 1;
    } else {
        // In air, rising, or no ground below
        self->position.z = predicted.z;
        if (pdata) pdata->onGround = 0;
    }

    // --- Update bounds ---
    self->bounds.x = self->position.x;
    self->bounds.y = self->position.y;
    self->bounds.z = self->position.z;

	
}



Uint8 entity_floor_check(Entity* self) {

}

void entity_draw(Entity* ent, GFC_Vector3D lightPos, GFC_Color colorMod) {
	GFC_Matrix4 modelMat;
	if (!ent) return;
	gfc_matrix4_from_vectors(modelMat, ent->position, ent->rotation, ent->scale);
	gf3d_mesh_draw(ent->mesh,
		modelMat,
		ent->color,
		ent->texture,
		lightPos,
		colorMod);
}

void entity_draw_shadow(Entity* ent) {
	GFC_Vector3D drawPosition;
	GFC_Matrix4 modelMat;
	if (!ent || !ent->drawShadow) return 0;
	gfc_vector3d_copy(drawPosition, ent->position);
	gfc_matrix4_from_vectors(modelMat, ent->position, ent->rotation, gfc_vector3d(ent->scale.x, ent->scale.y, .1));
}

void entity_draw_all(GFC_Vector3D lightPos, GFC_Color colorMod) {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (!entity_system.entity_list[i]._inuse) continue;
		entity_draw(&entity_system.entity_list[i], lightPos, colorMod);
	}
}

void entity_think(Entity* self)
{
	if (!self)return;
	if (self->think)self->think(self);
}

void entity_think_all() {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (!entity_system.entity_list[i]._inuse)continue;
		entity_think(&entity_system.entity_list[i]);
	}
}

void entity_update(Entity* self)
{
	if (!self)return;
	if (self->update)self->update(self);
}

void entity_update_all() {
	int i;
	for (i = 0; i < entity_system.entity_max; i++) {
		if (!entity_system.entity_list[i]._inuse)continue;
		entity_update(&entity_system.entity_list[i]);
	}
}