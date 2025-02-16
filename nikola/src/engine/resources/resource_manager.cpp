#include "nikola/nikola_core.hpp"
#include "nikola/nikola_engine.hpp"

#include "loaders/texture_loader.hpp"
#include "loaders/cubemap_loader.hpp"
#include "loaders/mesh_loader.hpp"
#include "loaders/material_loader.hpp"
#include "loaders/skybox_loader.hpp"

//////////////////////////////////////////////////////////////////////////

namespace nikola { // Start of nikola

/// ----------------------------------------------------------------------
/// StorageManager 
struct StorageManager {
  HashMap<String, ResourceStorage*> storages;

  ResourceStorage* cached_storage = nullptr;
  GfxContext* gfx_context         = nullptr;
};

static StorageManager s_manager;
/// StorageManager 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// ResourceStorage 
struct ResourceStorage {
  String name; 
  FilePath parent_dir;

  HashMap<ResourceID, GfxBuffer*> buffers;
  HashMap<ResourceID, GfxTexture*> textures;
  HashMap<ResourceID, GfxCubemap*> cubemaps;
  HashMap<ResourceID, GfxShader*> shaders;
  
  HashMap<ResourceID, Mesh*> meshes;
  HashMap<ResourceID, Material*> materials;
  HashMap<ResourceID, Skybox*> skyboxes;
  HashMap<ResourceID, Model*> models;
  HashMap<ResourceID, Font*> fonts;
};
/// ResourceStorage 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Macros (Unfortunately)

#define DESTROY_RESOURCE_MAP(storage, map, clear_func) { \
  for(auto& [key, value] : storage->map) {               \
    clear_func(value);                                   \
  }                                                      \
}

/// Macros (Unfortunately)
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions 

ResourceID generate_id() {
  return random_u64(); // @TODO: Make something more complex than this
}

template<typename T>
static T get_resource(ResourceStorage* storage, HashMap<ResourceID, T>& map, const ResourceID& id) {
  // We cannot return a non-existing resource neither will 
  // we add it to the storage 
  if(map.find(id) == map.end()) {
    NIKOLA_LOG_ERROR("Resource id \'%i\' does not exist in storage \'%s\'", id, storage->name.c_str());
    return nullptr; 
  }

  return map[id];
}

/// Private functions 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Resource manager functions

void resource_manager_init() {
  const GfxContext* gfx = renderer_get_context();
  NIKOLA_ASSERT(gfx, "Invalid graphics context passed to the resource manager");

  s_manager.gfx_context    = (GfxContext*)gfx;
  s_manager.cached_storage = resource_storage_create("cache", "resource_cache/");

  NIKOLA_LOG_INFO("Successfully initialized the resource manager");
}

void resource_manager_shutdown() {
  resource_storage_destroy(s_manager.cached_storage);
  
  // Get rid of any remaining storages
  for(auto& [key, value] : s_manager.storages) {
    resource_storage_destroy(value);
  }
  s_manager.storages.clear();
  
  NIKOLA_LOG_INFO("Successfully shutdown the resource manager");
}

const ResourceStorage* resource_manager_cache() {
  return s_manager.cached_storage;
}

/// Resource manager functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Resource storage functions

ResourceStorage* resource_storage_create(const String& name, const FilePath& parent_dir) {
  ResourceStorage* res = new ResourceStorage; //memory_allocate(sizeof(ResourceStorage));

  res->name                     = name; 
  res->parent_dir               = parent_dir;
  s_manager.storages[res->name] = res; 

  NIKOLA_LOG_INFO("Successfully created a resource storage \'%s\'", res->name.c_str());
  return res;
}

void resource_storage_clear(ResourceStorage* storage) {
  NIKOLA_ASSERT(storage, "Cannot clear an invalid storage");
  
  storage->buffers.clear();
  storage->textures.clear();
  storage->cubemaps.clear();
  storage->shaders.clear();
  storage->meshes.clear();
  storage->materials.clear();
  storage->skyboxes.clear();
  storage->models.clear();
  storage->fonts.clear();
  
  NIKOLA_LOG_INFO("Resource storage \'%s\' was successfully destroyed", storage->name.c_str());
}

void resource_storage_destroy(ResourceStorage* storage) {
  if(!storage) {
    return;
  }

  // Get rid of every resource in the storage
  DESTROY_RESOURCE_MAP(storage, buffers, gfx_buffer_destroy);
  DESTROY_RESOURCE_MAP(storage, textures, gfx_texture_destroy);
  DESTROY_RESOURCE_MAP(storage, cubemaps, gfx_cubemap_destroy);
  DESTROY_RESOURCE_MAP(storage, shaders, gfx_shader_destroy);

  s_manager.storages.erase(storage->name);
  delete storage; //memory_free(storage);
}

ResourceID resource_storage_push(ResourceStorage* storage, const GfxBufferDesc& buff_desc) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");

  ResourceID id        = generate_id();
  storage->buffers[id] = gfx_buffer_create(s_manager.gfx_context, buff_desc);
  
  return id;
}

ResourceID resource_storage_push(ResourceStorage* storage, const GfxTextureDesc& desc) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");

  ResourceID id         = generate_id();
  storage->textures[id] = gfx_texture_create(s_manager.gfx_context, desc);
  
  return id;
}

ResourceID resource_storage_push(ResourceStorage* storage, 
                                 const FilePath& nbr_path,
                                 const GfxTextureFormat format, 
                                 const GfxTextureFilter filter, 
                                 const GfxTextureWrap wrap) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");

  // Load the NBR file
  NBRFile nbr;
  nbr_file_load(&nbr, storage->parent_dir / nbr_path);

  // Make sure it is the correct resource type
  NIKOLA_ASSERT((nbr.resource_type == RESOURCE_TYPE_TEXTURE), "Expected RESOURCE_TYPE_TEXTURE");

  // Convert the NBR format to a valid texture
  GfxTextureDesc tex_desc = {};
  NBRTexture* nbr_texture = (NBRTexture*)nbr.body_data;
  texture_loader_load(&tex_desc, nbr_texture, format, filter, wrap);
 
  // Create the texture 
  ResourceID id         = generate_id();
  storage->textures[id] = gfx_texture_create(s_manager.gfx_context, tex_desc);

  // Remember to close the NBR
  nbr_file_unload(nbr);

  // New texture added!
  return id;
}

ResourceID resource_storage_push(ResourceStorage* storage, const GfxCubemapDesc& cubemap_desc) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");

  ResourceID id         = generate_id();
  storage->cubemaps[id] = gfx_cubemap_create(s_manager.gfx_context, cubemap_desc);
  
  return id;
}

ResourceID resource_storage_push(ResourceStorage* storage, 
                                 const FilePath& nbr_path,
                                 const sizei faces_count,
                                 const GfxTextureFormat format, 
                                 const GfxTextureFilter filter, 
                                 const GfxTextureWrap wrap) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");

  // Load the NBR file
  NBRFile nbr;
  nbr_file_load(&nbr, storage->parent_dir / nbr_path);

  // Make sure it is the correct resource type
  NIKOLA_ASSERT((nbr.resource_type == RESOURCE_TYPE_CUBEMAP), "Expected RESOURCE_TYPE_CUBEMAP");

  // Convert the NBR format to a valid cubemap
  GfxCubemapDesc cube_desc = {};
  NBRCubemap* nbr_cubemap = (NBRCubemap*)nbr.body_data;
  cubemap_loader_load(&cube_desc, nbr_cubemap);

  // Remember to close the NBR
  nbr_file_unload(nbr);

  // New cubemap added!
  return resource_storage_push(storage, cube_desc);
}

ResourceID resource_storage_push(ResourceStorage* storage, const String& shader_src) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");

  ResourceID id        = generate_id();
  storage->shaders[id] = gfx_shader_create(s_manager.gfx_context, shader_src.c_str());

  return id;
}

NIKOLA_API ResourceID resource_storage_push(ResourceStorage* storage, 
                                            const ResourceID& vertex_buffer_id, 
                                            const VertexType vertex_type, 
                                            const ResourceID& index_buffer_id, 
                                            const sizei indices_count) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");

  // Allocate the mesh
  Mesh* mesh = (Mesh*)memory_allocate(sizeof(Mesh));
  memory_zero(mesh, sizeof(Mesh));

  // Use the loader to set up the mesh
  mesh_loader_load(storage, mesh, vertex_buffer_id, vertex_type, index_buffer_id, indices_count);

  // Create the pipeline
  mesh->pipe = gfx_pipeline_create(s_manager.gfx_context, mesh->pipe_desc);

  // New mesh added!
  mesh->storage_ref   = storage; 
  ResourceID id       = generate_id();
  storage->meshes[id] = mesh;

  return id;
}

NIKOLA_API ResourceID resource_storage_push(ResourceStorage* storage, const MeshType type) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");

  // Allocate the mesh
  Mesh* mesh = (Mesh*)memory_allocate(sizeof(Mesh));
  memory_zero(mesh, sizeof(Mesh));

  // Use the loader to set up the mesh
  mesh_loader_load(storage, mesh, type);

  // Create the pipeline
  mesh->pipe = gfx_pipeline_create(s_manager.gfx_context, mesh->pipe_desc);

  // New mesh added!
  mesh->storage_ref   = storage; 
  ResourceID id       = generate_id();
  storage->meshes[id] = mesh;

  return id;
}

NIKOLA_API ResourceID resource_storage_push(ResourceStorage* storage,
                                            const ResourceID& diffuse_id, 
                                            const ResourceID& specular_id, 
                                            const ResourceID& shader_id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  
  // Allocate the material
  Material* material = (Material*)memory_allocate(sizeof(Material));
  memory_zero(material, sizeof(Material));

  // Use the loader to set up the material
  material_loader_load(storage, material, diffuse_id, specular_id, shader_id);

  // New material added!
  material->storage_ref  = storage; 
  ResourceID id          = generate_id();
  storage->materials[id] = material;

  return id;
}

NIKOLA_API ResourceID resource_storage_push(ResourceStorage* storage, const ResourceID& cubemap_id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");

  // Allocate the skybox
  Skybox* skybox = (Skybox*)memory_allocate(sizeof(Skybox));
  memory_zero(skybox, sizeof(Skybox));
  
  // Use the loader to set up the skybox
  skybox_loader_load(storage, skybox, cubemap_id);

  // Create the pipeline 
  skybox->pipe = gfx_pipeline_create(s_manager.gfx_context, skybox->pipe_desc);

  // New skybox added!
  skybox->storage_ref   = storage; 
  ResourceID id         = generate_id();
  storage->skyboxes[id] = skybox;

  return id;
}

GfxBuffer* resource_storage_get_buffer(ResourceStorage* storage, const ResourceID& id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  NIKOLA_ASSERT((id != INVALID_RESOURCE), "Cannot retrieve an invalid resource");

  return get_resource(storage, storage->buffers, id);
}

GfxTexture* resource_storage_get_texture(ResourceStorage* storage, const ResourceID& id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  NIKOLA_ASSERT((id != INVALID_RESOURCE), "Cannot retrieve an invalid resource");

  return get_resource(storage, storage->textures, id);
}

GfxCubemap* resource_storage_get_cubemap(ResourceStorage* storage, const ResourceID& id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  NIKOLA_ASSERT((id != INVALID_RESOURCE), "Cannot retrieve an invalid resource");

  return get_resource(storage, storage->cubemaps, id);
}

GfxShader* resource_storage_get_shader(ResourceStorage* storage, const ResourceID& id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  NIKOLA_ASSERT((id != INVALID_RESOURCE), "Cannot retrieve an invalid resource");

  return get_resource(storage, storage->shaders, id);
}

Mesh* resource_storage_get_mesh(ResourceStorage* storage, const ResourceID& id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  NIKOLA_ASSERT((id != INVALID_RESOURCE), "Cannot retrieve an invalid resource");

  return get_resource(storage, storage->meshes, id);
}

Material* resource_storage_get_material(ResourceStorage* storage, const ResourceID& id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  NIKOLA_ASSERT((id != INVALID_RESOURCE), "Cannot retrieve an invalid resource");

  return get_resource(storage, storage->materials, id);
}

Model* resource_storage_get_model(ResourceStorage* storage, const ResourceID& id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  NIKOLA_ASSERT((id != INVALID_RESOURCE), "Cannot retrieve an invalid resource");

  return get_resource(storage, storage->models, id);
}

Skybox* resource_storage_get_skybox(ResourceStorage* storage, const ResourceID& id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  NIKOLA_ASSERT((id != INVALID_RESOURCE), "Cannot retrieve an invalid resource");

  return get_resource(storage, storage->skyboxes, id);
}

Font* resource_storage_get_font(ResourceStorage* storage, const ResourceID& id) {
  NIKOLA_ASSERT(storage, "Cannot push a resource to an invalid storage");
  NIKOLA_ASSERT((id != INVALID_RESOURCE), "Cannot retrieve an invalid resource");

  return get_resource(storage, storage->fonts, id);
}

/// Resource storage functions
/// ----------------------------------------------------------------------

} // End of nikola

//////////////////////////////////////////////////////////////////////////
