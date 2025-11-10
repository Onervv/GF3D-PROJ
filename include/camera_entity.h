#ifndef __CAMERA_ENTITY_H__
#define __CAMERA_ENTITY_H__

#include "gfc_vector.h"
#include "gf3d_camera.h"
#include "entity.h"

typedef struct {
	GFC_Vector3D position;
	GFC_Vector3D target;
	float zOffset;
	Entity* player; // Pointer to player entity to follow
}CameraEntity;

CameraEntity* camera_entity_new();

void camera_think(CameraEntity* ce);

#endif