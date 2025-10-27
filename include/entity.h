#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_text.h"
#include "gfc_primitives.h"

#include "gf3d_mesh.h"

typedef struct Entity_S {
	Uint8 _inuse;
	GFC_TextLine name;
	Mesh* mesh;
	Texture* texture;
	GFC_Color color;
	GFC_Matrix4 matrix;
	GFC_Vector3D position;
	GFC_Vector3D rotation;
	GFC_Vector3D scale;
	GFC_Box bounds;
	GFC_Vector3D velocity;
	void (*draw)(struct Entity_S* self);
	void (*think)(struct Entity_S* self);
	void (*update)(struct Entity_S* self);
	void* data;
}Entity;

/*@brief creates a new blank entity
* @return NULL on error or no memory, otherwise the new entity
*/
Entity* entity_new();

/*@brief frees the passed entity
* @param e the entity to free
*/
void entity_free(Entity* e);

/*@brief initializes the entity manager
* @param max_ents the number of ents allocated in the manager
*/
void entity_system_init(Uint16 max_ents);

void entity_system_close();

void entity_move(Entity* self);

void entity_draw_all(GFC_Vector3D lightPos, GFC_Color colorMod);

void entity_think(Entity* self);

void entity_think_all();

void entity_update(Entity* self);

void entity_update_all();




#endif