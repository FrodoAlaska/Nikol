#include "nikola/nikola_core.hpp"
#include "nikola/nikola_engine.hpp"

//////////////////////////////////////////////////////////////////////////

namespace nikola {

///---------------------------------------------------------------------------------------------------------------------
/// Private functions

static void check_and_send_uniform(GfxShader* shader, const i8* name, GfxLayoutType type, const void* data) {
  i32 location = gfx_glsl_get_uniform_location(shader, name);

  // Could not find the uniform
  if(location == -1) {
    NIKOLA_LOG_WARN("Could not find uniform value \'%s\'", name);
    return;   
  }

  // Send the uniform over to the shader
  gfx_glsl_upload_uniform(shader, location, type, data);
}

static void check_and_send_preset_uniform(Material* mat, const i8* name, GfxLayoutType type, const void* data) {
  // Does not exist 
  if(mat->uniform_locations.find(name) == mat->uniform_locations.end()) {
    return;
  }
    
  // Send the uniform
  gfx_glsl_upload_uniform(mat->shader, mat->uniform_locations[name], type, data);
}

/// Private functions
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// Material functions

void material_set_uniform(Material* mat, const i8* uniform_name, const i32 value) {
  NIKOLA_ASSERT(mat, "Invalid Material passed");
  NIKOLA_ASSERT(mat->shader, "Invalid Material's shader");
  
  check_and_send_uniform(mat->shader, uniform_name, GFX_LAYOUT_INT1, &value);
}

void material_set_uniform(Material* mat, const i8* uniform_name, const f32 value) {
  NIKOLA_ASSERT(mat, "Invalid Material passed");
  NIKOLA_ASSERT(mat->shader, "Invalid Material's shader");
  
  check_and_send_uniform(mat->shader, uniform_name, GFX_LAYOUT_FLOAT1, &value);
}

void material_set_uniform(Material* mat, const i8* uniform_name, const Vec2& value) {
  NIKOLA_ASSERT(mat, "Invalid Material passed");
  NIKOLA_ASSERT(mat->shader, "Invalid Material's shader");
  
  check_and_send_uniform(mat->shader, uniform_name, GFX_LAYOUT_FLOAT2, &value[0]);
}

void material_set_uniform(Material* mat, const i8* uniform_name, const Vec3& value) {
  NIKOLA_ASSERT(mat, "Invalid Material passed");
  NIKOLA_ASSERT(mat->shader, "Invalid Material's shader");
  
  check_and_send_uniform(mat->shader, uniform_name, GFX_LAYOUT_FLOAT3, &value[0]);
}

void material_set_uniform(Material* mat, const i8* uniform_name, const Vec4& value) {
  NIKOLA_ASSERT(mat, "Invalid Material passed");
  NIKOLA_ASSERT(mat->shader, "Invalid Material's shader");
  
  check_and_send_uniform(mat->shader, uniform_name, GFX_LAYOUT_FLOAT4, &value[0]);
}

void material_set_uniform(Material* mat, const i8* uniform_name, const Mat4& value) {
  NIKOLA_ASSERT(mat, "Invalid Material passed");
  NIKOLA_ASSERT(mat->shader, "Invalid Material's shader");
  
  check_and_send_uniform(mat->shader, uniform_name, GFX_LAYOUT_MAT4, (void*)mat4_raw_data(value));
}

void material_set_uniform_buffer(Material* mat, const sizei index, GfxBuffer* buffer) {
  NIKOLA_ASSERT(buffer, "Cannot attach an invalid uniform buffer to a material");
  NIKOLA_ASSERT(((index >= MATERIAL_MATRICES_BUFFER_INDEX) && (index <= MATERIAL_LIGHTING_BUFFER_INDEX)), "Invalid index passed as uniform buffer index of a material");

  mat->uniform_buffers[index] = (GfxBuffer*)buffer;
  gfx_shader_attach_uniform(mat->shader, GFX_SHADER_VERTEX, mat->uniform_buffers[index], index);
}

void material_use(Material* mat) {
  NIKOLA_ASSERT(mat, "Invalid Material passed");
  NIKOLA_ASSERT(mat->shader, "Invalid Material's shader");

  // Send all of the available uniforms
  check_and_send_preset_uniform(mat, MATERIAL_UNIFORM_AMBIENT_COLOR, GFX_LAYOUT_FLOAT3, &mat->ambient_color[0]);
  check_and_send_preset_uniform(mat, MATERIAL_UNIFORM_DIFFUSE_COLOR, GFX_LAYOUT_FLOAT3, &mat->diffuse_color[0]);
  check_and_send_preset_uniform(mat, MATERIAL_UNIFORM_SPECULAR_COLOR, GFX_LAYOUT_FLOAT3, &mat->specular_color[0]);
  check_and_send_preset_uniform(mat, MATERIAL_UNIFORM_MODEL_MATRIX, GFX_LAYOUT_MAT4, mat4_raw_data(mat->model_matrix));
}

/// Material functions
///---------------------------------------------------------------------------------------------------------------------

} // End of nikola

//////////////////////////////////////////////////////////////////////////
