#ifndef __ENTITY_H__

#define __ENTITY_H__

#include "gfc_text.h"
#include "gfc_matrix.h"
#include "gf3d_mesh.h"
#include "gfc_vector.h"


typedef struct Entity_S{
    Uint8               _inuse;
    GFC_TextLine        name;

    Mesh               *mesh;
    Texture            *texture;
    GFC_Color          color;

    GFC_Matrix4        matrix;
    GFC_Vector3D       position;
    GFC_Vector3D       rotation;
    GFC_Vector3D       scale;

    GFC_Box            bounds;
    void               (*draw)(struct Entity_S *self);
    void               (*think)(struct Entity_S *self);
    void               (*update)(struct Entity_S *self);
    
} Entity;


/**
 * @brief: get a pointer to a new, blank entity
 * @return: NULL on out of memory or other orrer; a pointer to the blank entity otherwise.
*/
Entity *entitynew();

/**
 * @brief: free an entity previously allocated from the masterlist
 * @param: ent - pointer to the entity to free
 * @NOTE: the memory address should be set to NULL
*/
void entity_free(Entity *ent);  // use memset to clear mesh andtexture pointers

/**
 * @brief initalize the entity subsystem 
 * @param max_ents how many to support concurrently 
 */
void entity_system_init(Uint32 max_ents);

/**
 * @brief: draw all entitys in master list
*/
void entity_draw_all();

/**
 * @brief: draw an entity
 * @param: ent - pointer to ent to draw
*/
void entity_draw(Entity *ent);

void entity_think_all();

void entity_update_all();

#endif // __ENTITY_H__