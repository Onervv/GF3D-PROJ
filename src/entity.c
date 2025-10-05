#include "simple_logger.h"

#include "entity.h"


typedef struct {
    Entity *entity_list;
    Uint32  entity_max;

} EntitySystem;

static EntitySystem entity_system = {0};

void entity_system_close();

Entity* Entitynew(){
    int i;

    if (entity_system.entity_list){
        for (i = 0; i < entity_system.entity_max; i++){ // find a free one
            if (!entity_system.entity_list[i]._inuse){
                entity_system.entity_list[i]._inuse = 1;
                entity_system.entity_list[i].color = GFC_COLOR_WHITE;
                entity_system.entity_list[i].scale = gfc_vector3d(1,1,1); // normalize scale 
                
            }
        }
    }
    return NULL;
}

void entity_free(Entity *ent){
    if (!ent)return;
    
    if (ent->mesh){
        gf3d_mesh_free(ent->mesh);
    }

    if (ent->texture){
        gf3d_texture_free(ent->texture);
    }

    memset(ent,0,sizeof(Entity)); // free everything outside of conditionals above
}

void entity_system_close() {
	int i;

	if (entity_system.entity_list) {
		for (i = 0; i < entity_system.entity_max; i++) {
			if ( entity_system.entity_list[i]._inuse ) {
				entity_free( &entity_system.entity_list[i] );
			}
		}
	}
}

void entity_system_init(Uint32 max_ents) {
	if (!max_ents) {
		slog("Cannot initialize an entity system with zero entities");
	}

	entity_system.entity_list = gfc_allocate_array(sizeof(Entity), max_ents);

	if (!entity_system.entity_list) {
		slog("Failed to allocate %i entities", max_ents);
		return;
	}
	entity_system.entity_max = max_ents;
	atexit(entity_system_close);
}

//TODO: REFINE FUNCTIONS BELOW
void entity_draw(Entity *ent){
    if (!ent)return;
    if (ent->draw)ent->draw(ent);
}

void entity_system_draw_all(){
    int i;

    if (!entity_system.entity_list)return;
    for (i = 0; i < entity_system.entity_max; i++){
        if (entity_system.entity_list[i]._inuse){
            entity_draw(&entity_system.entity_list[i]);
        }
    }
}

void entity_think_all(){
    int i;

    if (!entity_system.entity_list)return;
    for (i = 0; i < entity_system.entity_max; i++){
        if (entity_system.entity_list[i]._inuse){
            if (entity_system.entity_list[i].think)entity_system.entity_list[i].think(&entity_system.entity_list[i]);
        }
    }
}

void entity_system_update_all(){
    int i;

    if (!entity_system.entity_list)return;
    for (i = 0; i < entity_system.entity_max; i++){
        if (entity_system.entity_list[i]._inuse){
            if (entity_system.entity_list[i].update)entity_system.entity_list[i].update(&entity_system.entity_list[i]);
        }
    }
}