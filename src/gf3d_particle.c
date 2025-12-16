#include "gf3d_particle.h"
#include "gf3d_pipeline.h"
#include "gf3d_vgraphics.h"
#include "gf3d_camera.h"
#include "gf3d_mesh.h"
#include "gf3d_buffers.h"
#include "simple_logger.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    GFC_Matrix4 model;
    GFC_Matrix4 view;
    GFC_Matrix4 proj;
    GFC_Vector4D color;
} ParticleUBO;

static ParticleSystem gf3d_particle_system = {0};

// Wireframe cube vertices for LINE_LIST (12 edges = 24 vertices)
static Vertex cube_vertices[] = {
    // Bottom face (Z = -0.5)
    {{-0.5, -0.5, -0.5}, {0, 0, -1}, {0, 0}}, {{ 0.5, -0.5, -0.5}, {0, 0, -1}, {0, 0}},  // Edge 1
    {{ 0.5, -0.5, -0.5}, {0, 0, -1}, {0, 0}}, {{ 0.5,  0.5, -0.5}, {0, 0, -1}, {0, 0}},  // Edge 2
    {{ 0.5,  0.5, -0.5}, {0, 0, -1}, {0, 0}}, {{-0.5,  0.5, -0.5}, {0, 0, -1}, {0, 0}},  // Edge 3
    {{-0.5,  0.5, -0.5}, {0, 0, -1}, {0, 0}}, {{-0.5, -0.5, -0.5}, {0, 0, -1}, {0, 0}},  // Edge 4
    
    // Top face (Z = 0.5)
    {{-0.5, -0.5,  0.5}, {0, 0, 1}, {0, 0}}, {{ 0.5, -0.5,  0.5}, {0, 0, 1}, {0, 0}},  // Edge 5
    {{ 0.5, -0.5,  0.5}, {0, 0, 1}, {0, 0}}, {{ 0.5,  0.5,  0.5}, {0, 0, 1}, {0, 0}},  // Edge 6
    {{ 0.5,  0.5,  0.5}, {0, 0, 1}, {0, 0}}, {{-0.5,  0.5,  0.5}, {0, 0, 1}, {0, 0}},  // Edge 7
    {{-0.5,  0.5,  0.5}, {0, 0, 1}, {0, 0}}, {{-0.5, -0.5,  0.5}, {0, 0, 1}, {0, 0}},  // Edge 8
    
    // Vertical edges
    {{-0.5, -0.5, -0.5}, {-1, -1, 0}, {0, 0}}, {{-0.5, -0.5,  0.5}, {-1, -1, 0}, {0, 0}},  // Edge 9
    {{ 0.5, -0.5, -0.5}, { 1, -1, 0}, {0, 0}}, {{ 0.5, -0.5,  0.5}, { 1, -1, 0}, {0, 0}},  // Edge 10
    {{ 0.5,  0.5, -0.5}, { 1,  1, 0}, {0, 0}}, {{ 0.5,  0.5,  0.5}, { 1,  1, 0}, {0, 0}},  // Edge 11
    {{-0.5,  0.5, -0.5}, {-1,  1, 0}, {0, 0}}, {{-0.5,  0.5,  0.5}, {-1,  1, 0}, {0, 0}}   // Edge 12
};

static Mesh* create_wireframe_cube() {
    Mesh* mesh = gf3d_mesh_new();
    if (!mesh) {
        slog("Failed to create particle mesh");
        return NULL;
    }
    
    MeshPrimitive* primitive = gfc_allocate_array(sizeof(MeshPrimitive), 1);
    if (!primitive) {
        slog("Failed to allocate primitive");
        return NULL;
    }
    
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    void* data = NULL;
    Uint32 vcount = 24;  // 12 edges × 2 vertices
    size_t bufferSize = sizeof(Vertex) * vcount;
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    // Create staging buffer
    gf3d_buffer_create(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer,
        &stagingBufferMemory
    );
    
    // Copy vertex data to staging buffer
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, cube_vertices, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    
    // Create device-local vertex buffer
    gf3d_buffer_create(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &primitive->vertexBuffer,
        &primitive->vertexBufferMemory
    );
    
    // Copy from staging to device buffer
    gf3d_buffer_copy(stagingBuffer, primitive->vertexBuffer, bufferSize);
    
    // Cleanup staging buffer
    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
    
    primitive->vertexCount = vcount;
    
    // No face buffer needed for LINE_LIST topology
    primitive->faceCount = 0;
    primitive->faceBuffer = VK_NULL_HANDLE;
    primitive->faceBufferMemory = VK_NULL_HANDLE;
    primitive->objData = NULL;
    
    // Add primitive to mesh
    mesh->primitives = gfc_list_new();
    gfc_list_append(mesh->primitives, primitive);
    
    gfc_line_cpy(mesh->filename, "particle_cube");
    mesh->_inuse = 1;
    mesh->_refCount = 1;
    
    slog("Wireframe cube created with %d vertices", primitive->vertexCount);
    
    return mesh;
}

void gf3d_particle_system_init(Uint32 maxParticles) {
    if (gf3d_particle_system.particles) {
        slog("Particle system already initialized");
        return;
    }
    
    memset(&gf3d_particle_system, 0, sizeof(ParticleSystem));
    
    gf3d_particle_system.maxParticles = maxParticles;
    gf3d_particle_system.particles = gfc_allocate_array(sizeof(Particle), maxParticles);
    
    if (!gf3d_particle_system.particles) {
        slog("Failed to allocate particles");
        return;
    }
    
    // Vertex input binding (same as your model system)
    VkVertexInputBindingDescription vertexInputDescription = {0};
    vertexInputDescription.binding = 0;
    vertexInputDescription.stride = sizeof(Vertex);
    vertexInputDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    // Vertex attributes (position, normal, texcoord - same as Vertex struct)
    VkVertexInputAttributeDescription vertexInputAttributeDescriptions[3];
    
    // Position
    vertexInputAttributeDescriptions[0].binding = 0;
    vertexInputAttributeDescriptions[0].location = 0;
    vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, vertex);
    
    // Normal
    vertexInputAttributeDescriptions[1].binding = 0;
    vertexInputAttributeDescriptions[1].location = 1;
    vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, normal);
    
    // Texcoord
    vertexInputAttributeDescriptions[2].binding = 0;
    vertexInputAttributeDescriptions[2].location = 2;
    vertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttributeDescriptions[2].offset = offsetof(Vertex, texel);
    
    // Get device and extent
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    VkExtent2D extent = gf3d_vgraphics_get_view_extent();
    
    // Load particle pipeline with all required parameters
    gf3d_particle_system.pipeline = gf3d_pipeline_create_from_config(
        device,
        "config/particle_pipeline.cfg",
        extent,
        1000,  // descriptorCount (max particles we can draw)
        &vertexInputDescription,
        vertexInputAttributeDescriptions,
        3,  // vertexAttributeCount (position, normal, texcoord)
        sizeof(ParticleUBO),  // bufferSize for UBO
        VK_INDEX_TYPE_UINT16  // Not used for LINE_LIST but required
    );
    
    if (!gf3d_particle_system.pipeline) {
        slog("Failed to load particle pipeline");
        return;
    }
    
    // Create wireframe cube mesh
    gf3d_particle_system.cubeMesh = create_wireframe_cube();
    if (!gf3d_particle_system.cubeMesh) {
        slog("Failed to create cube mesh");
        return;
    }
    
    slog("Particle system initialized with %d max particles", maxParticles);
}
   

void gf3d_particle_spawn(GFC_Vector3D position, GFC_Vector3D velocity, GFC_Color color, float life, float size) {
    if (!gf3d_particle_system.particles) return;
    
    // Find inactive particle
    for (Uint32 i = 0; i < gf3d_particle_system.maxParticles; i++) {
        Particle* p = &gf3d_particle_system.particles[i];
        if (!p->active) {
            p->position = position;
            p->velocity = velocity;
            p->color = color;
            p->life = life;
            p->maxLife = life;
            p->size = size;
            p->active = 1;
            
            gf3d_particle_system.particleCount++;
            return;
        }
    }
    
}

void gf3d_particle_update(float deltaTime) {
    if (!gf3d_particle_system.particles) return;
    
    for (Uint32 i = 0; i < gf3d_particle_system.maxParticles; i++) {
        Particle* p = &gf3d_particle_system.particles[i];
        if (!p->active) continue;
        
        // Update life
        p->life -= deltaTime;
        if (p->life <= 0) {
            p->active = 0;
            gf3d_particle_system.particleCount--;
            continue;
        }
        
        // Update position
        p->position.x += p->velocity.x * deltaTime;
        p->position.y += p->velocity.y * deltaTime;
        p->position.z += p->velocity.z * deltaTime;
        
        // Apply simple gravity
        p->velocity.z -= 2.0f * deltaTime;
        
        // Apply air resistance
        p->velocity.x *= 0.98f;
        p->velocity.y *= 0.98f;
        
        // Fade out based on remaining life
        float lifePercent = p->life / p->maxLife;
        p->color.a = lifePercent * 0.7f;  // Max 70% opacity for translucent effect
    }
}

void gf3d_particle_draw() {
    if (!gf3d_particle_system.particles) return;
    if (!gf3d_particle_system.pipeline) return;
    if (!gf3d_particle_system.cubeMesh) return;
    
    ParticleUBO ubo;
    
    // Get view and projection matrices
    gf3d_vgraphics_get_view(&ubo.view);
    gf3d_vgraphics_get_projection_matrix(&ubo.proj);
    
    for (Uint32 i = 0; i < gf3d_particle_system.maxParticles; i++) {
        Particle* p = &gf3d_particle_system.particles[i];
        if (!p->active) continue;
        
        // Create model matrix (position + scale)
        GFC_Matrix4 modelMat;
        gfc_matrix4_from_vectors(
            modelMat,
            p->position,
            gfc_vector3d(0, 0, 0),  // No rotation
            gfc_vector3d(p->size, p->size, p->size)
        );
        gfc_matrix4_copy(ubo.model, modelMat);
        
        // Set color with fading alpha
        ubo.color.x = p->color.r;
        ubo.color.y = p->color.g;
        ubo.color.z = p->color.b;
        ubo.color.w = p->color.a;
        
        // Queue particle for rendering (no texture needed)
        gf3d_mesh_queue_render(
            gf3d_particle_system.cubeMesh,
            gf3d_particle_system.pipeline,
            &ubo,
            NULL  // No texture
        );
    }
}

void gf3d_mesh_primitive_free(MeshPrimitive* primitive) {
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    
    if (!primitive) return;
    
    // Destroy Vulkan buffers BEFORE freeing the struct
    if (primitive->faceBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, primitive->faceBuffer, NULL);
    }
    if (primitive->faceBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, primitive->faceBufferMemory, NULL);
    }
    
    // Also clean up vertex buffers (you likely have these)
    if (primitive->vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, primitive->vertexBuffer, NULL);
    }
    if (primitive->vertexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, primitive->vertexBufferMemory, NULL);
    }
    
    // Free OBJ data
    if (primitive->objData) {
        gf3d_obj_free(primitive->objData);
    }
}

void gf3d_particle_system_close() {
    slog("Particle system closing - START");
    
    if (gf3d_particle_system.particles) {
        slog("Freeing particles array at %p", gf3d_particle_system.particles);
        free(gf3d_particle_system.particles);
        gf3d_particle_system.particles = NULL;
    }
    
    gf3d_particle_system.cubeMesh = NULL;
    gf3d_particle_system.pipeline = NULL;
    
    memset(&gf3d_particle_system, 0, sizeof(ParticleSystem));
    slog("Particle system closed - END");
}

void gf3d_particle_spawn_explosion(GFC_Vector3D position, int particleCount) {
    for (int i = 0; i < particleCount; i++) {
        gf3d_particle_spawn(
            position,
            gfc_vector3d(
                (rand() % 400 - 200) / 50.0f,  // X: -4 to 4
                (rand() % 400 - 200) / 50.0f,  // Y: -4 to 4
                (rand() % 300 + 100) / 50.0f   // Z: 2 to 8 (upward)
            ),
            gfc_color(
                0.8f + (rand() % 20) / 100.0f,   // Red: 0.8-1.0 (fire colors)
                0.3f + (rand() % 40) / 100.0f,   // Green: 0.3-0.7
                0.1f + (rand() % 20) / 100.0f,   // Blue: 0.1-0.3
                1.0f
            ),
            2.0f + (rand() % 100) / 100.0f,  // Life: 2-3 seconds
            0.4f + (rand() % 30) / 100.0f    // Size: 0.4-0.7
        );
    }
}

void gf3d_particle_update_rain(float deltaTime, GFC_Vector3D centerPosition) {
    static float rainSpawnTimer = 0.0f;
    const float spawnInterval = 0.05f;   // Spawn every 0.05 seconds
    const int particlesPerSpawn = 3;      // 3 particles each time
    const float rainRadius = 75.0f;       // 50-unit radius circle
    const float rainHeight = 30.0f;       // Spawn 30 units high
    
    rainSpawnTimer += deltaTime;
    
    // Spawn particles at intervals
    while (rainSpawnTimer >= spawnInterval) {
        rainSpawnTimer -= spawnInterval;
        
        for (int i = 0; i < particlesPerSpawn; i++) {
            // Random position in circle around center
            float angle = (rand() % 360) * (3.14159f / 180.0f);
            float distance = (rand() % 100) / 100.0f * rainRadius;
            
            GFC_Vector3D spawnPos = gfc_vector3d(
                centerPosition.x + cos(angle) * distance,
                centerPosition.y + sin(angle) * distance,
                rainHeight
            );
            
            // Downward velocity with slight drift
            GFC_Vector3D velocity = gfc_vector3d(
                (rand() % 40 - 20) / 100.0f,        // X: -0.2 to 0.2
                (rand() % 40 - 20) / 100.0f,        // Y: -0.2 to 0.2
                -8.0f - (rand() % 200) / 100.0f     // Z: -8 to -10 (falling)
            );
            
            // Blue/cyan rain colors
            GFC_Color rainColor = gfc_color(
                0.2f + (rand() % 30) / 100.0f,   // Red: 0.2-0.5
                0.5f + (rand() % 50) / 100.0f,   // Green: 0.5-1.0
                0.8f + (rand() % 20) / 100.0f,   // Blue: 0.8-1.0
                0.6f + (rand() % 40) / 100.0f    // Alpha: 0.6-1.0
            );
            
            gf3d_particle_spawn(
                spawnPos,
                velocity,
                rainColor,
                3.0f + (rand() % 100) / 100.0f,  // Life: 3-4 seconds
                0.2f + (rand() % 15) / 100.0f    // Size: 0.2-0.35
            );
        }
    }
}