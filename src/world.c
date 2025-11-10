#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"
#include "gfc_text.h"
#include "gf3d_obj_load.h"
#include "world.h"

static World* theWorld;

World* get_the_world() {
	if (!theWorld) {
		slog("No static world to return");
		return NULL;
	}
	return theWorld;
}

World* world_new() {
	World* world;
	world = gfc_allocate_array(sizeof(World), 1);
	if (!world)return NULL;

	return world;
}

World* world_load(const char* filename) {
	World* world;
	const char* str;
	SJson* json, *config;

	json = sj_load(filename);
	if (!json) {
		slog("Failed to load world file %s", filename);
		return NULL;
	}
	world = world_new();
	if (!world) {
		slog("Failed to allocate world for file %s", filename);
		return NULL;
	}
	config = sj_object_get_value(json, "world");
	str = sj_object_get_value_as_string(config, "filename");
	world->mesh = gf3d_mesh_load(str);
	str = sj_object_get_value_as_string(config, "texture");
	world->texture = gf3d_texture_load(str);
	sj_object_get_color_value(config, "color", &world->color);
	sj_object_get_vector3d(config, "lightPosition", &world->lightPosition);
	sj_free(json); // We need to free the json data after parsing
	theWorld = world;
	return world;
}

void world_free(World* w) {
	gf3d_mesh_free(w->mesh);
	gf3d_texture_free(w->texture);
	gfc_list_clear(w->entities);
	theWorld = NULL; 
	memset(w, 0, sizeof(World));
}

void world_draw(World* w) {
	GFC_Matrix4 id;
	gfc_matrix4_identity(id);
	gf3d_mesh_draw(w->mesh, id, GFC_COLOR_WHITE, w->texture, w->lightPosition, w->color);
}

Uint8 world_edge_test(World* world, GFC_Vector3D start, GFC_Vector3D end, GFC_Vector3D* contact) {
	int i, j, pCount, fCount;
	GFC_Edge3D edge;
	GFC_Triangle3D tri;
	MeshPrimitive* primitive;

	if (!world) return NULL;

	edge = gfc_edge3d_from_vectors(start, end);
	pCount = gfc_list_count(world->mesh->primitives);

	// check each primitive in the mesh
	// check each face in the primitive
	// if the edge intersects the face, return true
	for (i = 0; i < pCount; i++) {
		primitive = gfc_list_nth(world->mesh->primitives, i);
		if ((!primitive) || (!primitive->objData)) continue;
		fCount = primitive->objData->face_count;
		for (j = 0; j < fCount; j++) {
			tri.a = primitive->objData->faceVertices[primitive->objData->outFace[j].verts[0]].vertex;
			tri.b = primitive->objData->faceVertices[primitive->objData->outFace[j].verts[1]].vertex;
			tri.c = primitive->objData->faceVertices[primitive->objData->outFace[j].verts[2]].vertex;
			if (gfc_trigfc_angle_edge_test(edge, tri, contact)) {
				return 1; 
			}
		}
	}

	return 0;
}