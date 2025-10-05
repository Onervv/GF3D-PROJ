#include "simple_logger.h"
#include "simple_json.h"
#include "gf3d_vgraphics.h"
#include "gf3d_swapchain.h"
#include "gf3d_obj_load.h"
#include "gf3d_buffers.h"
#include "gf3d_mesh.h"
#include "gf3d_camera.h"
#include "gf3d_pipeline.h"
#include "gf3d_texture.h"
#include "gfc_list.h"
#include <string.h>
#include <stdlib.h>

#define MESH_ATTRIBUTE_COUNT 3

extern int __DEBUG;

typedef struct
{
    Mesh                   *mesh_list;
    Uint32                  mesh_count;
    Uint32                  chain_length;
    VkDevice                device;
    Pipeline               *pipe;
    VkVertexInputAttributeDescription   attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription     bindingDescription;
    Texture        *defaultTexture;
} MeshManager;

static MeshManager mesh_manager = {0};

void gf3d_mesh_manager_close();
void gf3d_mesh_delete(Mesh* mesh);
void gf3d_mesh_primitive_free(MeshPrimitive* prim);
void gf3d_mesh_primitive_create_vertex_buffers(MeshPrimitive* prim);
void gf3d_mesh_primitive_setup_face_buffers(MeshPrimitive* prim);

void gf3d_mesh_delete(Mesh* mesh)
{
    if (!mesh) return;
    memset(mesh, 0, sizeof(Mesh));
}

void gf3d_mesh_manager_close()
{
    int i;
    for (i = 0; i < mesh_manager.mesh_count; i++)
    {
        gf3d_mesh_delete(&mesh_manager.mesh_list[i]);
    }
    if (mesh_manager.mesh_list) free(mesh_manager.mesh_list);
    if (mesh_manager.pipe) gf3d_pipeline_free(mesh_manager.pipe);
    if (mesh_manager.defaultTexture) gf3d_texture_free(mesh_manager.defaultTexture);
    memset(&mesh_manager, 0, sizeof(MeshManager));
    if (__DEBUG) slog("mesh manager closed");
}

VkVertexInputAttributeDescription* gf3d_mesh_get_attribute_descriptions(Uint32* count)
{
    mesh_manager.attributeDescriptions[0].binding = 0;
    mesh_manager.attributeDescriptions[0].location = 0;
    mesh_manager.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[0].offset = offsetof(Vertex, vertex);

    mesh_manager.attributeDescriptions[1].binding = 0;
    mesh_manager.attributeDescriptions[1].location = 1;
    mesh_manager.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    mesh_manager.attributeDescriptions[1].offset = offsetof(Vertex, normal);

    mesh_manager.attributeDescriptions[2].binding = 0;
    mesh_manager.attributeDescriptions[2].location = 2;
    mesh_manager.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    mesh_manager.attributeDescriptions[2].offset = offsetof(Vertex, texel);

    if (count) *count = MESH_ATTRIBUTE_COUNT;
    return mesh_manager.attributeDescriptions;
}

VkVertexInputBindingDescription* gf3d_mesh_get_bind_description()
{
    mesh_manager.bindingDescription.binding = 0;
    mesh_manager.bindingDescription.stride = sizeof(Vertex);
    mesh_manager.bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return &mesh_manager.bindingDescription;
}

void gf3d_mesh_init(Uint32 mesh_max)
{
    Uint32 count;
    if (mesh_max == 0)
    {
        slog("cannot initialize mesh manager for 0 meshes");
        return;
    }

    mesh_manager.chain_length = gf3d_swapchain_get_chain_length();
    mesh_manager.mesh_list = (Mesh*)gfc_allocate_array(sizeof(Mesh), mesh_max);
    mesh_manager.mesh_count = mesh_max;
    mesh_manager.device = gf3d_vgraphics_get_default_logical_device();

    gf3d_mesh_get_attribute_descriptions(&count);
    mesh_manager.pipe = gf3d_pipeline_create_from_config(
        gf3d_vgraphics_get_default_logical_device(),
        "config/model_pipeline.cfg",
        gf3d_vgraphics_get_view_extent(),
        mesh_max,
        gf3d_mesh_get_bind_description(),
        gf3d_mesh_get_attribute_descriptions(NULL),
        count,
        sizeof(MeshUBO),
        VK_INDEX_TYPE_UINT16
    );

    mesh_manager.defaultTexture = gf3d_texture_load("images/default.png");
    if (__DEBUG) slog("mesh manager initialized");
    atexit(gf3d_mesh_manager_close);
}

MeshPrimitive* gf3d_mesh_primitive_new()
{
    return gfc_allocate_array(sizeof(MeshPrimitive), 1);
}

void gf3d_mesh_primitive_free(MeshPrimitive* prim)
{
    if (!prim) return;
    gf3d_obj_free(prim->objData);

    if (prim->vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->vertexBuffer, NULL);
        prim->vertexBuffer = VK_NULL_HANDLE;
    }
    if (prim->vertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mesh_manager.device, prim->vertexBufferMemory, NULL);
        prim->vertexBufferMemory = VK_NULL_HANDLE;
    }
    if (prim->faceBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(mesh_manager.device, prim->faceBuffer, NULL);
        prim->faceBuffer = VK_NULL_HANDLE;
    }
    if (prim->faceBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(mesh_manager.device, prim->faceBufferMemory, NULL);
        prim->faceBufferMemory = VK_NULL_HANDLE;
    }
    free(prim);
}

void gf3d_mesh_primitive_create_vertex_buffers(MeshPrimitive* prim)
{
    if (!prim) return;

    void* data = NULL;
    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    Vertex* vertices = prim->objData->faceVertices;
    Uint32 vcount = prim->objData->face_vert_count;
    size_t bufferSize = sizeof(Vertex) * vcount;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &prim->vertexBuffer, &prim->vertexBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    prim->vertexCount = vcount;
}

void gf3d_mesh_primitive_setup_face_buffers(MeshPrimitive* prim)
{
    if (!prim || !prim->objData) return;

    void* data = NULL;
    Face* faces = prim->objData->outFace;
    Uint32 fcount = prim->objData->face_count;
    if (!faces || !fcount) return;

    VkDevice device = gf3d_vgraphics_get_default_logical_device();
    VkDeviceSize bufferSize = sizeof(Face) * fcount;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, faces, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    gf3d_buffer_create(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &prim->faceBuffer, &prim->faceBufferMemory);

    gf3d_buffer_copy(stagingBuffer, prim->faceBuffer, bufferSize);

    prim->faceCount = fcount;
    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
}

Mesh* gf3d_mesh_new()
{
    for (int i = 0; i < mesh_manager.mesh_count; i++)
    {
        if (mesh_manager.mesh_list[i]._refCount) continue;
        memset(&mesh_manager.mesh_list[i], 0, sizeof(Mesh));
        mesh_manager.mesh_list[i]._refCount = 1;
        mesh_manager.mesh_list[i].primitives = gfc_list_new();
        return &mesh_manager.mesh_list[i];
    }
    return NULL;
}

void gf3d_mesh_free(Mesh* mesh)
{
    if (!mesh) return;

    if (mesh->primitives)
    {
        int c = gfc_list_count(mesh->primitives);
        for (int i = 0; i < c; i++)
        {
            MeshPrimitive* prim = gfc_list_nth(mesh->primitives, i);
            gf3d_mesh_primitive_free(prim);
        }
        gfc_list_delete(mesh->primitives);
    }

    mesh->_refCount--;
    if (mesh->_refCount <= 0) gf3d_mesh_delete(mesh);
}

Mesh* gf3d_mesh_get_by_filename()
{
    for (int i = 0; i < mesh_manager.mesh_count; i++)
    {
        if (mesh_manager.mesh_list[i]._refCount) continue;
    }
    return NULL;
}

Mesh* gf3d_mesh_load_obj(const char* filename)
{
    if (!filename) return NULL;

    ObjData* obj = gf3d_obj_load_from_file(filename);
    if (!obj)
    {
        slog("failed to parse obj file %s", filename);
        return NULL;
    }

    Mesh* mesh = gf3d_mesh_new();
    if (!mesh)
    {
        slog("failed to allocate mesh for file %s", filename);
        gf3d_obj_free(obj);
        return NULL;
    }

    gfc_line_cpy(mesh->filename, filename);
    MeshPrimitive* primitive = gf3d_mesh_primitive_new();
    if (!primitive)
    {
        slog("failed to allocate primitive for file %s", filename);
        gf3d_obj_free(obj);
        gf3d_mesh_free(mesh);
        return NULL;
    }

    primitive->objData = obj;
    gf3d_mesh_primitive_create_vertex_buffers(primitive);
    gf3d_mesh_primitive_setup_face_buffers(primitive);
    gfc_list_append(mesh->primitives, primitive);

    return mesh;
}

Pipeline* gf3d_mesh_get_pipeline()
{
    if (mesh_manager.pipe) return mesh_manager.pipe;
    slog("mesh pipe was not loaded");
    return NULL;
}

void gf3d_mesh_primitive_queue_render(MeshPrimitive* prim, Pipeline* pipe, void* uboData, Texture* texture)
{
    if (!prim || !pipe || !uboData) return;
    if (!texture) texture = mesh_manager.defaultTexture;
    gf3d_pipeline_queue_render(pipe, prim->vertexBuffer, prim->vertexCount, prim->faceBuffer, uboData, texture);
}

void gf3d_mesh_queue_render(Mesh* mesh, Pipeline* pipe, void* uboData, Texture* texture)
{
    if (!mesh || !pipe || !uboData) return;

    int c = gfc_list_count(mesh->primitives);
    for (int i = 0; i < c; i++)
    {
        MeshPrimitive* prim = gfc_list_nth(mesh->primitives, i);
        if (!prim) continue;
        gf3d_mesh_primitive_queue_render(prim, pipe, uboData, texture);
    }
}

void gf3d_mesh_draw(Mesh* mesh, GFC_Matrix4 modelMat, GFC_Color mod, Texture* texture)
{
    if (!mesh) return;
    MeshUBO ubo = {0};
    gfc_matrix4_copy(ubo.model, modelMat);
    gf3d_vgraphics_get_view(&ubo.view);
    gf3d_vgraphics_get_projection_matrix(&ubo.proj);
    ubo.color = gfc_color_to_vector4f(mod);
    gf3d_mesh_queue_render(mesh, mesh_manager.pipe, &ubo, texture);
}