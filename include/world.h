#ifndef __WORLD_H__
#define __WORLD_H__

#include "gf3d_mesh.h"
#include "entity.h"

typedef struct {
    Mesh*           mesh;
    Texture*        texture;
    GFC_List*       entities;
    GFC_Color       color;
    GFC_Vector3D    lightPosition;
} World;

World* get_the_world();

World* world_new();

World* world_load(const char* filename);

void world_free(World* w);

void world_draw(World* w);

Uint8 world_edge_test(World* world, GFC_Vector3D start, GFC_Vector3D end, GFC_Vector3D* contact);

#endif