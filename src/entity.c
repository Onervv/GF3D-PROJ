#include "simple_logger.h"

#include "entity.h"
#include "world.h"

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
	GFC_Box bounds;
	GFC_Vector3D positionPre, positionPost, contact;
	GFC_Vector2D direction2d;

	direction2d = gfc_vector2d_from_angle(self->rotation.z);
	gfc_vector2d_normalize(&direction2d);

	gfc_vector3d_copy(positionPre, self->position);
	gfc_vector3d_add(positionPost, self->position, self->velocity);

	if (world_edge_test(get_the_world(), positionPre, positionPost, &contact)) {
		slog("CONTACT %f, %f, %f", contact.x, contact.y, contact.z);
		
	}
	else {
		gfc_vector3d_copy(self->position, positionPost);
	}
	
	gfc_box_cpy(bounds, self->bounds); //start of collision checking
	gfc_vector3d_add(bounds, bounds, self->velocity);
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