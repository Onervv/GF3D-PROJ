#ifndef __CAMERA_ENTITY_H__
#define __CAMERA_ENTITY_H__

#include "gfc_vector.h"
#include "gf3d_camera.h"

typedef struct {
	GFC_Vector3D position;
	GFC_Vector3D target;
}CameraEntity;

CameraEntity* camera_entity_new();

void camera_think(CameraEntity* ce);

#endif