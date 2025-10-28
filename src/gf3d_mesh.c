#include <stdalign.h>

#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_shape.h"

#include "gf3d_buffers.h"
#include "gf3d_swapchain.h"
#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_commands.h"
#include "gf3d_mesh.h"
#include "gf3d_obj_load.h"
#include "gf3d_camera.h"

#define MESH_ATTRIBUTE_COUNT 3

extern int __DEBUG;

typedef struct {
    Mesh*       mesh_list;
    Uint32      max_meshes;
    Uint32      chain_length;
    VkDevice    device;
    Pipeline*   pipe;
    VkBuffer    faceBuffer;
    VkDeviceMemory  faceBufferMemory;
    VkVertexInputAttributeDescription   attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
    float       drawOrder;
    Texture* defaultTexture;
}MeshManager;

static MeshManager gf3d_mesh = { 0 };

void gf3d_mesh_init(Uint32 meshMax) { 
    Uint32 count;

    slog("Initializing Mesh Manager");
    if (!meshMax) {
        slog("Cannot initialize mesh system with 0 meshes");
        return;
    }
    gf3d_mesh.mesh_list = gfc_allocate_array(sizeof(Mesh), meshMax);
    if (!gf3d_mesh.mesh_list)
    {
        slog("failed to allocate %i meshes", meshMax);
        return;
    }
    gf3d_mesh.max_meshes = meshMax;
    gf3d_mesh.chain_length = gf3d_swapchain_get_chain_length();
    gf3d_mesh.mesh_list = (Mesh*)gfc_allocate_array(sizeof(Mesh), meshMax);
    gf3d_mesh.device = gf3d_vgraphics_get_default_logical_device();

    gf3d_mesh_get_attribute_descriptions(&count);
    // Duble check this
    gf3d_mesh.pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/model_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        meshMax,
        gf3d_mesh_get_bind_description(),
        gf3d_mesh_get_attribute_descriptions(NULL),
        count,
        sizeof(MeshUBO),
        VK_INDEX_TYPE_UINT16);
    slog("mesh system initialized");
}

// Populate this
void gf3d_mesh_close(){ 

}

Mesh* gf3d_mesh_new() {
    int i;
    for (i = 0; i < gf3d_mesh.max_meshes; i++)
    {
        if (gf3d_mesh.mesh_list[i]._inuse)continue;
        gf3d_mesh.mesh_list[i]._inuse = 1;
        return &gf3d_mesh.mesh_list[i];
    }
    slog("gf3d_mesh_new: no free slots for new meshes");
    return NULL;
}

Mesh* gf3d_mesh_load(const char* filename) {
    ObjData* objectData;
    MeshPrimitive* prim;
    Mesh* mesh;
    
    if (!filename) return NULL;
    slog("loading object data...");
    objectData = gf3d_obj_load_from_file(filename); 
    if (!objectData) { 
        slog("failed to parse obj file %s", filename); slog_sync();
        return NULL;
    }
    slog("Object data loaded");
    mesh = gf3d_mesh_new();
    if (!mesh) {
        slog("failed to allocate mesh for file %s", filename); slog_sync();
        gf3d_obj_free(objectData);
        return NULL;
    }
    slog("New mesh allocated");
    prim = gf3d_mesh_primitive_new();
    if (!prim) {
        slog("failed to allocate mesh primitive for file %s", filename); slog_sync();
        gf3d_obj_free(objectData);
        gf3d_mesh_free(mesh);
        return NULL;
    }
    slog("primitive initialized"); slog_sync();
    mesh->primitives = gfc_list_new();
    gfc_list_append(mesh->primitives, prim);
    prim->objData = objectData;
    slog("Primitive appended to list with object data"); slog_sync();
    gf3d_mesh_primitive_create_vertex_buffer(prim);
    gf3d_mesh_primitive_create_face_buffer(prim); 
    
    return mesh;
}

void gf3d_mesh_draw(Mesh* mesh, GFC_Matrix4 modelMat, GFC_Color mod, Texture* texture) {
    MeshUBO ubo = { 0 };
    if (!mesh) return;

    //ubo = gf3d_mesh_get_ubo(modelMat, mod);
    gfc_matrix4_copy(ubo.model, modelMat);
    gf3d_vgraphics_get_view(&ubo.view);
    gf3d_vgraphics_get_projection_matrix(&ubo.proj);
    ubo.color = gfc_color_to_vector4f(mod);
    ubo.camera = gfc_vector3dw(gf3d_camera_get_position(), 1.0);
    gfc_matrix4_slog(modelMat);
    gf3d_mesh_queue_render(mesh, gf3d_mesh.pipe, &ubo, texture);
}

void gf3d_mesh_queue_render(Mesh* mesh, Pipeline* pipe, void* uboData, Texture* texture) {
    int i, c;
    MeshPrimitive *primitive;
    if (!mesh) {
        slog("Failed to queue mesh for render, NULL mesh");
        return;
    }
    if (!pipe) {
        slog("Failed to queue mesh for render, NULL pipeline");
        return;
    }
    if (!uboData) {
        slog("Failed to queue mesh for render, NULL ubo");
        return;
    }
    c = gfc_list_count(mesh->primitives);
    for (i = 0; i < c; i++) {
        primitive = gfc_list_nth(mesh->primitives, i);
        if (!primitive) continue;
        gf3d_mesh_primitive_queue_render(primitive, pipe, uboData, texture);
    }
}

void gf3d_mesh_primitive_queue_render(MeshPrimitive* primitive, Pipeline* pipe, void* uboData, Texture* texture) {
    if (!primitive || !pipe || !uboData) return NULL;
    if (!texture) texture = gf3d_mesh.defaultTexture;
    gf3d_pipeline_queue_render(pipe, primitive->vertexBuffer, primitive->vertexCount, primitive->faceBuffer, uboData, texture);
}

MeshPrimitive* gf3d_mesh_primitive_new() {
    return gfc_allocate_array(sizeof(MeshPrimitive), 1);
}

VkVertexInputAttributeDescription* gf3d_mesh_get_attribute_descriptions(Uint32* count) {
    gf3d_mesh.attributeDescriptions[0].binding = 0;
    gf3d_mesh.attributeDescriptions[0].location = 0;
    gf3d_mesh.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_mesh.attributeDescriptions[0].offset = offsetof(Vertex, vertex);

    gf3d_mesh.attributeDescriptions[1].binding = 0;
    gf3d_mesh.attributeDescriptions[1].location = 1;
    gf3d_mesh.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    gf3d_mesh.attributeDescriptions[1].offset = offsetof(Vertex, normal);

    gf3d_mesh.attributeDescriptions[2].binding = 0;
    gf3d_mesh.attributeDescriptions[2].location = 2;
    gf3d_mesh.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    gf3d_mesh.attributeDescriptions[2].offset = offsetof(Vertex, texel);

    if (count)*count = MESH_ATTRIBUTE_COUNT;
    slog("Done attribute desc");
    return gf3d_mesh.attributeDescriptions;
}

VkVertexInputBindingDescription* gf3d_mesh_get_bind_description() {
    gf3d_mesh.bindingDescription.binding = 0;
    gf3d_mesh.bindingDescription.stride = sizeof(Vertex);
    gf3d_mesh.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    slog("Done bind desc");
    return &gf3d_mesh.bindingDescription;
}

void gf3d_mesh_free(Mesh* mesh) {
    /*
    GFC_TextLine        filename;
    Uint32              _refCount;
    Uint8               _inuse;
    GFC_List* primitives;
    GFC_Box             bounds; */

    gfc_list_delete(mesh->primitives);
    free(mesh); // we free 
}

void gf3d_mesh_primitive_create_vertex_buffer(MeshPrimitive* primitive) {
    void* data = NULL;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();

    Vertex* vertices;
    Uint32 vcount;

    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (!primitive) {
        slog("no mesh primitive given to create vertex buffers for");
        return;
    }
 
    vertices = primitive->objData->faceVertices;
    vcount = primitive->objData->face_vert_count;
    bufferSize = sizeof(Vertex) * vcount;
    // STAGING 
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &primitive->vertexBuffer, &primitive->vertexBufferMemory);

    gf3d_buffer_copy(stagingBuffer, primitive->vertexBuffer, bufferSize);
    // After Copy Destory Buffer
    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    primitive->vertexCount = vcount;
    slog("Vertex buffer created");
}

void gf3d_mesh_primitive_create_face_buffer(MeshPrimitive* primitive) {
    void* data = NULL;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();

    Face* faces;
    Uint32 fcount;

    size_t bufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if (!primitive) {
        slog("no mesh primitive given to create vertex buffers for");
        return;
    }

    faces = primitive->objData->outFace; 
    fcount = primitive->objData->face_count;
    bufferSize = sizeof(Face) * fcount;
    //create staging data
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, faces, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    //copy staged data to the primitive
    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &primitive->faceBuffer, &primitive->faceBufferMemory);
    gf3d_buffer_copy(stagingBuffer, primitive->faceBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    primitive->faceCount = fcount;
    slog("Face buffer created");
}

Pipeline* gf3d_mesh_get_pipeline() {
    return gf3d_mesh.pipe;
}

MeshUBO gf3d_mesh_get_ubo(GFC_Matrix4 modelMat, GFC_Color colorMod) {
    ModelViewProjection mvp;
    MeshUBO ubo = { 0 };

    GFC_Vector4D color = gfc_color_to_vector4(colorMod);
    mvp = gf3d_vgraphics_get_mvp();

    gfc_matrix4_copy(ubo.model, modelMat);
    gfc_matrix4_copy(ubo.view, mvp.view);
    gfc_matrix4_copy(ubo.proj, mvp.proj);
    gfc_vector4d_copy(ubo.color, color);
    ubo.camera = gfc_vector3dw(gf3d_camera_get_position(), 1.0);

    return ubo;
}