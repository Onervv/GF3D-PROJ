#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_text.h"
#include "gfc_primitives.h"

#include "gf3d_mesh.h"

#define PLAYER_RADIUS        	1.0f    // radius of your sphere player
#define DOWN_RAY_ABOVE       	1.0f    // start ray slightly above the sphere's bottom
#define DOWN_RAY_BELOW       	2.5f    // HOLY SHIT MAGIC RAY CAST FLOATING POINT SAVIOR, keep this on 2.5
#define PENETRATION_TOLERANCE 	0.05f   // buffer to prevent jitter when snapping to ground
#define REST_TOLERANCE       	0.02f   // how close to ground before stopping Z velocity
#define GRAVITY                -0.02f   // downward acceleration per frame
// #define JUMP_STRENGTH        	0.5f    // initial velocity when jumping
// #define MOVE_SPEED           	0.05f   // forward/backward movement speed

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
	Uint8 drawShadow;
	void (*draw)(struct Entity_S* self);
	void (*think)(struct Entity_S* self, float deltaTime);   
    void (*update)(struct Entity_S* self, float deltaTime);  
	void (*free)(struct Entity_S* self);
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
* @param maxEnts the number of ents allocated in the manager
*/
void entity_system_init(Uint16 maxEnts);

void entity_system_close();

void entity_move(Entity* self);

void entity_draw_all(GFC_Vector3D lightPos, GFC_Color colorMod);

void entity_think(Entity* self, float deltaTime);

void entity_think_all(float deltaTime);

void entity_update(Entity* self, float deltaTime);

void entity_update_all(float deltaTime);




#endif