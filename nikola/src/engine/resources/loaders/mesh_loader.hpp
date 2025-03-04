#pragma once

#include "nikola/nikola_engine.hpp"

//////////////////////////////////////////////////////////////////////////

namespace nikola { // Start of nikola

void mesh_loader_load(ResourceStorage* storage, 
                      Mesh* mesh, 
                      const ResourceID& vertex_buffer_id, 
                      const VertexType vertex_type, 
                      const ResourceID& index_buffer_id, 
                      const sizei indices_count);

void mesh_loader_load(ResourceStorage* storage, Mesh* mesh, const MeshType type);

} // End of nikola

//////////////////////////////////////////////////////////////////////////
