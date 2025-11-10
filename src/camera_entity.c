#include "simple_logger.h"

#include "camera_entity.h"

CameraEntity* camera_entity_new() {
	CameraEntity* ce;
	ce = gfc_allocate_array(sizeof(CameraEntity), 1);

	return ce;
}

void camera_think(CameraEntity* ce) {

}