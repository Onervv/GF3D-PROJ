#ifndef __GF3D_PARTICLE_H__
#define __GF3D_PARTICLE_H__

#include "gfc_vector.h"
#include "gfc_color.h"
#include "gfc_matrix.h"
#include "gf3d_pipeline.h"
#include "gf3d_mesh.h"

typedef struct {
    GFC_Vector3D position;
    GFC_Vector3D velocity;
    GFC_Color color;
    float life;         // Time remaining (seconds)
    float maxLife;      // Total lifetime
    float size;         // Cube size
    Uint8 active;
} Particle;

typedef struct {
    Particle* particles;
    Uint32 maxParticles;
    Uint32 particleCount;
    Pipeline* pipeline;
    Mesh* cubeMesh;  // Wireframe cube
} ParticleSystem;

/**
 * @brief Initialize the particle system
 * @param maxParticles Maximum number of particles
 */
void gf3d_particle_system_init(Uint32 maxParticles);

/**
 * @brief Spawn a particle at a position
 * @param position Where to spawn
 * @param velocity Initial velocity
 * @param color Particle color
 * @param life How long it lives (seconds)
 * @param size Size of the cube
 */
void gf3d_particle_spawn(GFC_Vector3D position, GFC_Vector3D velocity, GFC_Color color, float life, float size);

/**
 * @brief Update all particles
 * @param deltaTime Time elapsed
 */
void gf3d_particle_update(float deltaTime);

/**
 * @brief Draw all particles
 */
void gf3d_particle_draw();

/**
 * @brief Cleanup particle system
 */
void gf3d_particle_system_close();

/**
 * @brief Update rain effect (spawns rain particles automatically)
 * @param deltaTime Time elapsed
 * @param centerPosition Center of rain circle (usually player position)
 */
void gf3d_particle_update_rain(float deltaTime, GFC_Vector3D centerPosition);

void gf3d_particle_spawn_explosion(GFC_Vector3D position, int particleCount);

#endif