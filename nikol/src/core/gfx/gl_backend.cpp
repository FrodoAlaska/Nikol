#include "nikol/nikol_core.hpp"

//////////////////////////////////////////////////////////////////////////

#ifdef NIKOL_GFX_CONTEXT_OPENGL  // OpenGL check

#include <glad/glad.h>

#include <cstring>

namespace nikol { // Start of nikol

/// ---------------------------------------------------------------------
/// *** Graphics ***

///---------------------------------------------------------------------------------------------------------------------
/// Macros

#define SET_GFX_STATE(value, state) { \
  if(value) {                         \
    glEnable(state);                  \
  }                                   \
  else {                              \
    glDisable(state);                 \
  }                                   \
}

#define SET_BUFFER_BIT(value, bits, buffer) { \
  if(value) {                                 \
    SET_BIT(bits, buffer);                    \
  }                                           \
  else {                                      \
    UNSET_BIT(bits, buffer);                  \
  }                                           \
}

/// Macros
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// GfxContext
struct GfxContext {
  GfxContextDesc desc;

  GfxStates states;
  sizei buffer_bits = 0;

  bool has_vsync = false;
};
/// GfxContext
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// GfxBuffer  
struct GfxBuffer {
  GfxBufferDesc desc;

  u32 id;

  GLenum gl_buff_type; 
  GLenum gl_buff_usage;
};
/// GfxBuffer  
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// GfxShader
struct GfxShader {
  u32 id, vert_id, frag_id;

  i8* vert_src; 
  i8* frag_src;
  
  i32 vert_src_len, frag_src_len;

  GfxBuffer* vertex_uniforms[UNIFORM_BUFFERS_MAX];
  GfxBuffer* fragment_uniforms[UNIFORM_BUFFERS_MAX];

  sizei vertex_uniforms_count   = 0; 
  sizei fragment_uniforms_count = 0;
};
/// GfxShader
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// GfxTexture
struct GfxTexture {
  GfxTextureDesc desc;

  u32 id;
};
/// GfxTexture
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// GfxPipeline
struct GfxPipeline {
  GfxPipelineDesc desc;

  u32 vertex_array;

  GfxBuffer* vertex_buffer = nullptr;
  sizei vertex_count       = 0;

  GfxBuffer* index_buffer  = nullptr; 
  sizei index_count        = 0;

  GfxDrawMode draw_mode;

  GfxShader* shader;

  u32 textures[TEXTURES_MAX] = {};
  sizei textures_count       = 0;
};
/// GfxPipeline
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// Callbacks 

static bool framebuffer_resize(const Event& event, const void* disp, const void* list) {
  if(event.type != EVENT_WINDOW_FRAMEBUFFER_RESIZED) {
    return false;
  }

  glViewport(0, 0, event.window_framebuffer_width, event.window_framebuffer_height);

  return true;
}

/// Callbacks 
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// Private functions 

static void check_supported_gl_version(const i32 major, const i32 minor) {
  NIKOL_ASSERT((major >= NIKOL_GL_MINIMUM_MAJOR_VERSION) && (minor >= NIKOL_GL_MINIMUM_MINOR_VERSION), 
               "OpenGL versions less than 4.2 are not supported");
}

static void gl_check_error(const i8* func_name) {
  GLenum err = glGetError(); 

  switch(err) {
    case GL_INVALID_ENUM: 
      NIKOL_LOG_ERROR("GL_ERROR: FUNC = %s, ERR = GL_INVALID_ENUM", func_name);
      break;
    case GL_INVALID_VALUE: 
      NIKOL_LOG_ERROR("GL_ERROR: FUNC = %s, ERR = GL_INVALID_VALUE", func_name);
      break;
    case GL_INVALID_OPERATION: 
      NIKOL_LOG_ERROR("GL_ERROR: FUNC = %s, ERR = GL_INVALID_OPERATION", func_name);
      break;
    case GL_STACK_OVERFLOW: 
      NIKOL_LOG_ERROR("GL_ERROR: FUNC = %s, ERR = GL_STACK_OVERFLOW", func_name);
      break;
    case GL_STACK_UNDERFLOW: 
      NIKOL_LOG_ERROR("GL_ERROR: FUNC = %s, ERR = GL_STACK_UNDERFLOW", func_name);
      break;
    case GL_OUT_OF_MEMORY: 
      NIKOL_LOG_ERROR("GL_ERROR: FUNC = %s, ERR = GL_OUT_OF_MEMORY", func_name);
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION: 
      NIKOL_LOG_ERROR("GL_ERROR: FUNC = %s, ERR = GL_INVALID_FRAMEBUFFER_OPERATION", func_name);
      break;
    case GL_CONTEXT_LOST: 
      NIKOL_LOG_ERROR("GL_ERROR: FUNC = %s, ERR = GL_CONTEXT_LOST", func_name);
      break;
    case GL_NO_ERROR:
      break;
  }
}

static GLenum get_gl_compare_func(const GfxCompareFunc func) {
  switch(func) {
    case GFX_COMPARE_ALWAYS:
      return GL_ALWAYS;
    case GFX_COMPARE_NEVER:
      return GL_NEVER;
    case GFX_COMPARE_LESS:
      return GL_LESS;
    case GFX_COMPARE_LESS_EQUAL:
      return GL_LEQUAL;
    case GFX_COMPARE_GREATER:
      return GL_GREATER;
    case GFX_COMPARE_GREATER_EQUAL:
      return GL_GEQUAL;
    case GFX_COMPARE_NOT_EQUAL:
      return GL_NOTEQUAL;
    default:
      return 0;
  } 
}

static GLenum get_gl_operation(const GfxOperation op) {
  switch(op) {
    case GFX_OP_KEEP:
      return GL_KEEP;
    case GFX_OP_ZERO:
      return GL_ZERO;
    case GFX_OP_INVERT:
      return GL_INVERT;
    case GFX_OP_REPLACE:
      return GL_REPLACE;
    case GFX_OP_INCR:
      return GL_INCR;
    case GFX_OP_DECR:
      return GL_DECR;
    case GFX_OP_INCR_WRAP:
      return GL_INCR_WRAP;
    case GFX_OP_DECR_WRAP:
      return GL_DECR_WRAP;
    default:
      return 0;
  }
}

static GLenum get_gl_blend_mode(const GfxBlendMode mode) {
  switch(mode) {
    case GFX_BLEND_ZERO:
      return GL_ZERO;
    case GFX_BLEND_ONE:
      return GL_ONE;
    case GFX_BLEND_SRC_COLOR:
      return GL_SRC_COLOR;
    case GFX_BLEND_DEST_COLOR:
      return GL_DST_COLOR;
    case GFX_BLEND_SRC_ALPHA:
      return GL_SRC_ALPHA;
    case GFX_BLEND_DEST_ALPHA:
      return GL_DST_ALPHA;
    case GFX_BLEND_INV_SRC_COLOR:
      return GL_ONE_MINUS_SRC_COLOR;
    case GFX_BLEND_INV_DEST_COLOR:
      return GL_ONE_MINUS_DST_COLOR;
    case GFX_BLEND_INV_SRC_ALPHA:
      return GL_ONE_MINUS_SRC_ALPHA;
    case GFX_BLEND_INV_DEST_ALPHA:
      return GL_ONE_MINUS_DST_ALPHA;
    case GFX_BLEND_SRC_ALPHA_SATURATE:
      return GL_SRC_ALPHA_SATURATE;
    default:
      return 0;
  }
}

static GLenum get_gl_cull_order(const GfxCullOrder order) {
  switch(order) {
    case GFX_ORDER_CLOCKWISE:
      return GL_CW;
    case GFX_ORDER_COUNTER_CLOCKWISE:
      return GL_CCW;
    default:
      return 0;
  }
}

static GLenum get_gl_cull_mode(const GfxCullMode mode) {
  switch(mode) {
    case GFX_CULL_FRONT:
      return GL_FRONT;
    case GFX_CULL_BACK:
      return GL_BACK;
    case GFX_CULL_FRONT_AND_BACK:
      return GL_FRONT_AND_BACK;
    default:
      return 0;
  }
}

static void set_state(GfxContext* gfx, const GfxStates state, const bool value) {
  switch(state) {
    case GFX_STATE_DEPTH:
      SET_GFX_STATE(value, GL_DEPTH_TEST);
      SET_BUFFER_BIT(value, gfx->buffer_bits, GL_DEPTH_BUFFER_BIT);
      break;
    case GFX_STATE_STENCIL:
      SET_GFX_STATE(value, GL_STENCIL_TEST);
      SET_BUFFER_BIT(value, gfx->buffer_bits, GL_STENCIL_BUFFER_BIT);
      break;
    case GFX_STATE_BLEND:
      SET_GFX_STATE(value, GL_BLEND);
      break;
    case GFX_STATE_MSAA:
      SET_GFX_STATE(value, GL_MULTISAMPLE);
      break;
    case GFX_STATE_CULL:
      SET_GFX_STATE(value, GL_CULL_FACE);
      break;
    case GFX_STATE_VSYNC:
      gfx->has_vsync = value; 
      break;
  }
}

static void set_depth_state(GfxContext* gfx) {
  set_state(gfx, GFX_STATE_DEPTH, true);   

  GLenum func = get_gl_compare_func(gfx->desc.depth_desc.compare_func);

  glDepthFunc(func);
  glDepthMask(gfx->desc.depth_desc.depth_write_enabled);
}

static void set_stencil_state(GfxContext* gfx) {
  set_state(gfx, GFX_STATE_STENCIL, true);   

  GLenum func  = get_gl_compare_func(gfx->desc.stencil_desc.compare_func);
  GLenum face  = get_gl_cull_mode(gfx->desc.stencil_desc.polygon_face); 
  GLenum sfail = get_gl_operation(gfx->desc.stencil_desc.stencil_fail_op); 
  GLenum dfail = get_gl_operation(gfx->desc.stencil_desc.depth_fail_op); 
  GLenum dpass = get_gl_operation(gfx->desc.stencil_desc.depth_pass_op); 

  glStencilFuncSeparate(face, func, gfx->desc.stencil_desc.ref, gfx->desc.stencil_desc.mask);
  glStencilOpSeparate(face, sfail, dfail, dpass);
  glStencilMaskSeparate(face, gfx->desc.stencil_desc.mask);
}

static void set_blend_state(GfxContext* gfx) {
  set_state(gfx, GFX_STATE_BLEND, true);   

  GLenum src_color = get_gl_blend_mode(gfx->desc.blend_desc.src_color_blend);
  GLenum dst_color = get_gl_blend_mode(gfx->desc.blend_desc.dest_color_blend);

  GLenum src_alpha = get_gl_blend_mode(gfx->desc.blend_desc.src_alpha_blend);
  GLenum dst_alpha = get_gl_blend_mode(gfx->desc.blend_desc.dest_alpha_blend);

  f32* factor = gfx->desc.blend_desc.blend_factor;

  glBlendFuncSeparate(src_color, dst_color, src_alpha, dst_alpha);
  glBlendColor(factor[0], factor[1], factor[2], factor[3]);
}

static void set_cull_state(GfxContext* gfx) {
  set_state(gfx, GFX_STATE_CULL, true);   

  GLenum front_face = get_gl_cull_order(gfx->desc.cull_desc.front_face);
  GLenum face       = get_gl_cull_mode(gfx->desc.cull_desc.cull_mode);
  
  glCullFace(face);
  glFrontFace(front_face);
}

static void set_gfx_states(GfxContext* gfx) {
  if(IS_BIT_SET(gfx->states, GFX_STATE_DEPTH)) {
    set_depth_state(gfx);
  }
  
  if(IS_BIT_SET(gfx->states, GFX_STATE_STENCIL)) {
    set_stencil_state(gfx);
  }
  
  if(IS_BIT_SET(gfx->states, GFX_STATE_BLEND)) {
    set_blend_state(gfx);
  }
  
  if(IS_BIT_SET(gfx->states, GFX_STATE_MSAA)) {
    set_state(gfx, GFX_STATE_MSAA, true);   
  }

  if(IS_BIT_SET(gfx->states, GFX_STATE_CULL)) {
    set_cull_state(gfx);
  }

  if(IS_BIT_SET(gfx->states, GFX_STATE_VSYNC)) {
    set_state(gfx, GFX_STATE_VSYNC, true);
  }
}

static GLenum get_buffer_type(const GfxBufferType type) {
  switch(type) {
    case GFX_BUFFER_VERTEX:
      return GL_ARRAY_BUFFER;
    case GFX_BUFFER_INDEX:
      return GL_ELEMENT_ARRAY_BUFFER;
    case GFX_BUFFER_UNIFORM:
      return GL_UNIFORM_BUFFER;
  } 
}

static GLenum get_buffer_usage(const GfxBufferUsage usage) {
  switch(usage) {
    case GFX_BUFFER_USAGE_DYNAMIC_DRAW:
      return GL_DYNAMIC_DRAW;
    case GFX_BUFFER_USAGE_DYNAMIC_READ:
      return GL_DYNAMIC_READ;
    case GFX_BUFFER_USAGE_STATIC_DRAW:
      return GL_STATIC_DRAW;
    case GFX_BUFFER_USAGE_STATIC_READ:
      return GL_STATIC_READ;
    default:
      return 0;
  }
}

static GLenum get_draw_mode(const GfxDrawMode mode) {
  switch(mode) {
    case GFX_DRAW_MODE_POINT:
      return GL_POINTS;
    case GFX_DRAW_MODE_TRIANGLE:
      return GL_TRIANGLES;
    case GFX_DRAW_MODE_TRIANGLE_STRIP:
      return GL_TRIANGLE_STRIP;
    case GFX_DRAW_MODE_LINE:
      return GL_LINES;
    case GFX_DRAW_MODE_LINE_STRIP:
      return GL_LINE_STRIP;
    default:
      return 0;
  }
}

static sizei get_layout_size(const GfxLayoutType layout) {
  switch(layout) {
    case GFX_LAYOUT_FLOAT1:
      return sizeof(f32);
    case GFX_LAYOUT_FLOAT2:
      return sizeof(f32) * 2;
    case GFX_LAYOUT_FLOAT3:
      return sizeof(f32) * 3;
    case GFX_LAYOUT_FLOAT4:
      return sizeof(f32) * 4;
    case GFX_LAYOUT_INT1:
      return sizeof(i32);
    case GFX_LAYOUT_INT2:
      return sizeof(i32) * 2;
    case GFX_LAYOUT_INT3:
      return sizeof(i32) * 3;
    case GFX_LAYOUT_INT4:
      return sizeof(i32) * 4;
    case GFX_LAYOUT_UINT1:
      return sizeof(u32);
    case GFX_LAYOUT_UINT2:
      return sizeof(u32) * 2;
    case GFX_LAYOUT_UINT3:
      return sizeof(u32) * 3;
    case GFX_LAYOUT_UINT4:
      return sizeof(u32) * 4;
    case GFX_LAYOUT_MAT2:
      return sizeof(f32) * 4;
    case GFX_LAYOUT_MAT3:
      return sizeof(f32) * 9;
    case GFX_LAYOUT_MAT4:
      return sizeof(f32) * 16;
    default: 
      return 0;
  }
}

static sizei get_layout_type(const GfxLayoutType layout) {
  switch(layout) {
    case GFX_LAYOUT_FLOAT1:
    case GFX_LAYOUT_FLOAT2:
    case GFX_LAYOUT_FLOAT3:
    case GFX_LAYOUT_FLOAT4:
    case GFX_LAYOUT_MAT2:
    case GFX_LAYOUT_MAT3:
    case GFX_LAYOUT_MAT4:
      return GL_FLOAT;
    case GFX_LAYOUT_INT1:
    case GFX_LAYOUT_INT2:
    case GFX_LAYOUT_INT3:
    case GFX_LAYOUT_INT4:
      return GL_INT;
    case GFX_LAYOUT_UINT1:
    case GFX_LAYOUT_UINT2:
    case GFX_LAYOUT_UINT3:
    case GFX_LAYOUT_UINT4:
      return GL_UNSIGNED_INT;
    default:
      return 0;
  }
}

static sizei get_layout_count(const GfxLayoutType layout) {
  switch(layout) {
    case GFX_LAYOUT_FLOAT1:
    case GFX_LAYOUT_INT1:
    case GFX_LAYOUT_UINT1:
      return 1;
    case GFX_LAYOUT_FLOAT2:
    case GFX_LAYOUT_INT2:
    case GFX_LAYOUT_UINT2:
    case GFX_LAYOUT_MAT2:
      return 2;
    case GFX_LAYOUT_FLOAT3:
    case GFX_LAYOUT_INT3:
    case GFX_LAYOUT_UINT3:
    case GFX_LAYOUT_MAT3:
      return 3;
    case GFX_LAYOUT_FLOAT4:
    case GFX_LAYOUT_INT4:
    case GFX_LAYOUT_UINT4:
    case GFX_LAYOUT_MAT4:
      return 4;
    default:
      return 0;
  }
}

static sizei get_semantic_count(const GfxLayoutType layout) {
  switch(layout) {
    case GFX_LAYOUT_MAT2:
      return 2;
    case GFX_LAYOUT_MAT3:
      return 3;
    case GFX_LAYOUT_MAT4:
      return 4;
    default:
      return 1;
  }
}

static sizei get_semantic_size(const GfxLayoutType layout) {
  switch(layout) {
    case GFX_LAYOUT_MAT2:
      return sizeof(f32) * 2;
    case GFX_LAYOUT_MAT3:
      return sizeof(f32) * 3;
    case GFX_LAYOUT_MAT4:
      return sizeof(f32) * 4;
    default: // Other types don't have semantics 
      return 0;
  }
}

static sizei calc_stride(const GfxLayoutDesc* layout, const sizei count) {
  sizei stride = 0; 

  for(sizei i = 0; i < count; i++) {
    stride += get_layout_size(layout[i].type);
  }

  return stride;
}

static bool is_semantic_attrib(const GfxLayoutType layout) {
  return layout == GFX_LAYOUT_MAT2 || 
         layout == GFX_LAYOUT_MAT3 || 
         layout == GFX_LAYOUT_MAT4;
}

static void set_vertex_attrib(const u32 vao, const GfxLayoutDesc& layout, const sizei index, sizei* offset) {
  glEnableVertexArrayAttrib(vao, index);
  gl_check_error("glEnableVertexArrayAttrib");

  GLenum gl_comp_type = get_layout_type(layout.type);
  sizei comp_count    = get_layout_count(layout.type);
  sizei size          = get_layout_size(layout.type);

  glVertexArrayAttribFormat(vao, index, comp_count, gl_comp_type, false, *offset);
  gl_check_error("glVertexArrayAttribIFormat");
  
  glVertexArrayBindingDivisor(vao, index, layout.instance_rate);
  gl_check_error("glVertexArrayBindingDivisor");
  
  glVertexArrayAttribBinding(vao, index, 0);
  gl_check_error("glVertexArrayAttribBinding");

  *offset += size;
}

static sizei set_semantic_attrib(const u32 vao, const GfxLayoutDesc& layout, const sizei index, sizei* offset) {
  sizei semantic_count = get_semantic_count(layout.type);  
  sizei semantic_size  = get_semantic_size(layout.type); 
  sizei semantic_index = 0;
  
  GLenum gl_comp_type = get_layout_type(layout.type);
  sizei comp_count    = get_layout_count(layout.type);

  for(sizei j = 0; j < semantic_count; j++) {
    semantic_index = j + index;

    glEnableVertexArrayAttrib(vao, semantic_index);
  
    glVertexArrayAttribFormat(vao, semantic_index, comp_count, gl_comp_type, false, *offset);
    glVertexArrayBindingDivisor(vao, semantic_index, layout.instance_rate);
    glVertexArrayAttribBinding(vao, semantic_index, 0);

    *offset += semantic_size;
  }

  return semantic_index;
}

static sizei set_buffer_layout(const u32 vao, const GfxLayoutDesc* layout, const sizei layout_count) {
  sizei stride         = calc_stride(layout, layout_count);
  sizei offset         = 0;
  sizei semantic_index = 0;

  for(sizei i = 0; i < layout_count; i++) {
    /// @NOTE: A "semantic" is the inner value of an attribute. 
    /// For example, the semantic of a 'Mat4' is just a `Vec4` or, rather, 
    /// 4 `Vec4`s. In that case `semantic_count` would be `4` and `semantic_size`
    /// would be `16` since it's just a `FLOAT4` under the hood.
    
    // Different configuration if the current layout is a semantic or not
    if(is_semantic_attrib(layout[i].type)) {
      semantic_index = set_semantic_attrib(vao, layout[i], semantic_index, &offset); 
    }
    else {
      set_vertex_attrib(vao, layout[i], semantic_index, &offset);
    }

    semantic_index += 1;
  }
  
  return stride;
}

static void seperate_shader_src(const i8* src, GfxShader* shader) {
  sizei src_len = strlen(src);
  
  sizei vert_start = 0;
  sizei frag_start = 0;

  // Find the vertex start
  for(sizei i = 0; i < src_len; i++) {
    if(src[i] != '#') { 
      continue;
    }

    // We found the start
    vert_start = i; 
    break;
  }

  // Find the frag start.
  // There's no point going from the beginning of the string. 
  // Just start from where we last stopped.
  for(sizei i = vert_start + 1; i < src_len; i++) {
    if(src[i] != '#') { 
      continue;
    }

    // We found the start
    frag_start = i; 
    break;
  }

  // Calculate where each shader ends
  sizei frag_end = src_len;

  shader->frag_src_len = frag_end - frag_start;
  shader->vert_src_len = frag_start - 1;

  // // Allocate new strings
  shader->vert_src = (i8*)memory_allocate(shader->vert_src_len);
  memory_zero(shader->vert_src, shader->vert_src_len);

  shader->frag_src = (i8*)memory_allocate(shader->frag_src_len);
  memory_zero(shader->frag_src, shader->frag_src_len);

  // Copying the correct strings over 
  memory_copy(shader->vert_src, src, shader->vert_src_len);
  memory_copy(shader->frag_src, &src[frag_start], shader->frag_src_len);
}

static void check_shader_compile_error(const sizei shader) {
  i32 success;
  i8 log_info[512];

  glGetShaderiv(shader, GL_COMPILE_STATUS, &success); 

  if(!success) {
    glGetShaderInfoLog(shader, 512, nullptr, log_info);
    NIKOL_LOG_WARN("SHADER-ERROR: %s", log_info);
  }
}

static void check_shader_linker_error(const GfxShader* shader) {
  i32 success;
  i8 log_info[512];

  glGetProgramiv(shader->id, GL_LINK_STATUS, &success); 

  if(!success) {
    glGetProgramInfoLog(shader->id, 512, nullptr, log_info);
    NIKOL_LOG_WARN("SHADER-ERROR: %s", log_info);
  }
}

static void get_texture_format(const GfxTextureFormat format, GLenum* in_format, GLenum* base_format) {
  switch(format) {
    case GFX_TEXTURE_FORMAT_R8:
      *in_format   = GL_R8;
      *base_format = GL_RED; 
    case GFX_TEXTURE_FORMAT_R16:
      *in_format   = GL_R16;
      *base_format = GL_RED; 
    case GFX_TEXTURE_FORMAT_RG8:
      *in_format   = GL_RG8;
      *base_format = GL_RG; 
    case GFX_TEXTURE_FORMAT_RG16:
      *in_format   = GL_RG16;
      *base_format = GL_RG; 
    case GFX_TEXTURE_FORMAT_RGBA8:
      *in_format   = GL_RGBA8;
      *base_format = GL_RGBA; 
    case GFX_TEXTURE_FORMAT_RGBA16:
      *in_format   = GL_RGBA16;
      *base_format = GL_RGBA; 
    default:
      break;
  }
}

static void get_texture_filter(const GfxTextureFilter filter, GLenum* min, GLenum* mag) {
  switch(filter) {
    case GFX_TEXTURE_FILTER_MIN_MAG_LINEAR:
      *min = GL_LINEAR; 
      *mag = GL_LINEAR;
      break;
    case GFX_TEXTURE_FILTER_MIN_MAG_NEAREST:
      *min = GL_NEAREST; 
      *mag = GL_NEAREST;
      break;
    case GFX_TEXTURE_FILTER_MIN_LINEAR_MAG_NEAREST:
      *min = GL_LINEAR; 
      *mag = GL_NEAREST;
      break;
    case GFX_TEXTURE_FILTER_MIN_NEAREST_MAG_LINEAR:
      *min = GL_NEAREST; 
      *mag = GL_LINEAR;
      break;
    case GFX_TEXTURE_FILTER_MIN_TRILINEAR_MAG_LINEAR:
      *min = GL_LINEAR_MIPMAP_LINEAR; 
      *mag = GL_LINEAR;
      break;
    case GFX_TEXTURE_FILTER_MIN_TRILINEAR_MAG_NEAREST:
      *min = GL_LINEAR_MIPMAP_LINEAR; 
      *mag = GL_NEAREST;
      break;
  }
}

static GLenum get_texture_wrap(const GfxTextureWrap wrap) {
  switch(wrap) {
    case GFX_TEXTURE_WRAP_REPEAT: 
      return GL_REPEAT;
    case GFX_TEXTURE_WRAP_MIRROR: 
      return GL_MIRRORED_REPEAT;
    case GFX_TEXTURE_WRAP_CLAMP: 
      return GL_CLAMP_TO_EDGE;
    case GFX_TEXTURE_WRAP_BORDER_COLOR:
      return GL_CLAMP_TO_BORDER;
    default:
      return 0;
  }
}

static bool is_buffer_dynamic(const GfxBufferUsage& usage) {
  return usage == GFX_BUFFER_USAGE_DYNAMIC_DRAW || 
         usage == GFX_BUFFER_USAGE_DYNAMIC_READ;
}

static void send_vertex_uniform_buffers(GfxShader* shader) {
  if(shader->vertex_uniforms_count <= 0) {
    return;
  }

  for(sizei i = 0; i < shader->vertex_uniforms_count; i++) {
    GfxBuffer* buff = shader->vertex_uniforms[i];
    glNamedBufferSubData(buff->id, 0, buff->desc.size, buff->desc.data);
  }
}

static void send_fragment_uniform_buffers(GfxShader* shader) {
  if(shader->fragment_uniforms_count <= 0) {
    return;
  }

  for(sizei i = 0; i < shader->fragment_uniforms_count; i++) {
    GfxBuffer* buff = shader->fragment_uniforms[i];
    glNamedBufferSubData(buff->id, 0, buff->desc.size, buff->desc.data);
  }
}

/// Private functions 
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// Context functions 

GfxContext* gfx_context_init(const GfxContextDesc& desc) {
  GfxContext* gfx = (GfxContext*)memory_allocate(sizeof(GfxContext));
  memory_zero(gfx, sizeof(GfxContext)); 
  
  gfx->desc        = desc;
  gfx->buffer_bits = GL_COLOR_BUFFER_BIT;

  // Glad init
  if(!gladLoadGL()) {
    NIKOL_LOG_FATAL("Could not create an OpenGL instance");
    return nullptr;
  }
  
  // Setting the window context to this OpenGL context 
  window_set_current_context(desc.window);

  // Setting up the viewport for OpenGL
  i32 width, height; 
  window_get_size(desc.window, &width, &height);
  glViewport(0, 0, width, height);

  // Setting the flags
  gfx->states = (GfxStates)desc.states;
  set_gfx_states(gfx);

  // Listening to events 
  event_listen(EVENT_WINDOW_FRAMEBUFFER_RESIZED, framebuffer_resize);

  // Getting some OpenGL information
  const u8* vendor       = glGetString(GL_VENDOR); 
  const u8* renderer     = glGetString(GL_RENDERER); 
  const u8* gl_version   = glGetString(GL_VERSION);
  const u8* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

  // Getting the version number
  i32 major_ver, minor_ver;
  glGetIntegerv(GL_MAJOR_VERSION, &major_ver);
  glGetIntegerv(GL_MINOR_VERSION, &minor_ver);
  check_supported_gl_version(major_ver, minor_ver);

  NIKOL_LOG_INFO("An OpenGL graphics context was successfully created:\n" 
                 "              VENDOR: %s\n" 
                 "              RENDERER: %s\n" 
                 "              GL VERSION: %s\n" 
                 "              GLSL VERSION: %s", 
                 vendor, renderer, gl_version, glsl_version);
  return gfx;
}

void gfx_context_shutdown(GfxContext* gfx) {
  if(!gfx) {
    return;
  }

  NIKOL_LOG_INFO("The graphics context was successfully destroyed");
  memory_free(gfx);
}

void gfx_context_clear(GfxContext* gfx, const f32 r, const f32 g, const f32 b, const f32 a) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");

  glClear(gfx->buffer_bits);
  glClearColor(r, g, b, a);
}

void gfx_context_present(GfxContext* gfx) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");
  window_swap_buffers(gfx->desc.window);
}

void gfx_context_set_state(GfxContext* gfx, const GfxStates state, const bool value) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");
  set_state(gfx, state, value);
}

void gfx_context_apply_pipeline(GfxContext* gfx, GfxPipeline* pipeline, const GfxPipelineDesc& pipe_desc) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");
  NIKOL_ASSERT(pipeline, "Invalid GfxPipeline struct passed");

  // Updating the pipeline 
  pipeline->desc = pipe_desc;
 
  // Updating the shader
  pipeline->shader = pipe_desc.shader;

  // Updating the textures
  pipeline->textures_count = pipe_desc.textures_count; 
  if(pipeline->textures_count > 0) {
    for(sizei i = 0; i < pipeline->textures_count; i++) {
      pipeline->textures[i] = pipe_desc.textures[i]->id;
    }
  }

  // Setting the stencil mask of the pipeline state
  glStencilMask(pipe_desc.stencil_ref);

  // Setting the blend color of the pipeline state
  const f32* blend_color = pipe_desc.blend_factor;
  glBlendColor(blend_color[0], blend_color[1], blend_color[2], blend_color[3]);

  // Bind the vertex array
  glBindVertexArray(pipeline->vertex_array);
}

/// Context functions 
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// Buffer functions 

GfxBuffer* gfx_buffer_create(GfxContext* gfx, const GfxBufferDesc& desc) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");

  GfxBuffer* buff = (GfxBuffer*)memory_allocate(sizeof(GfxBuffer));
  memory_zero(buff, sizeof(GfxBuffer));
  
  buff->desc            = desc;
  buff->gl_buff_type    = get_buffer_type(desc.type);
  buff->gl_buff_usage   = get_buffer_usage(desc.usage);

  glCreateBuffers(1, &buff->id);
  glNamedBufferData(buff->id, desc.size, desc.data, buff->gl_buff_usage);
  
  buff->desc = desc;
  return buff;
}

void gfx_buffer_destroy(GfxBuffer* buff) {
  if(!buff) {
    return;
  }

  glDeleteBuffers(1, &buff->id);
  memory_free(buff);
}

void gfx_buffer_update(GfxContext* gfx, GfxBuffer* buff, const sizei offset, const sizei size, const void* data) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");
  NIKOL_ASSERT(buff, "Invalid GfxBuffer struct passed");

  buff->desc.size = size;
  buff->desc.data = (void*)data;

  glNamedBufferSubData(buff->id, offset, size, data);
}

/// Buffer functions 
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// Shader functions 

GfxShader* gfx_shader_create(GfxContext* gfx, const i8* src) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");
  
  GfxShader* shader = (GfxShader*)memory_allocate(sizeof(GfxShader));
  memory_zero(shader, sizeof(GfxShader));

  seperate_shader_src(src, shader);

  // Vertex shader
  shader->vert_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shader->vert_id, 1, &shader->vert_src, &shader->vert_src_len); 
  glCompileShader(shader->vert_id);
  check_shader_compile_error(shader->vert_id);
    
  // Fragment shader
  shader->frag_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shader->frag_id, 1, &shader->frag_src, &shader->frag_src_len); 
  glCompileShader(shader->frag_id);
  check_shader_compile_error(shader->frag_id);

  // Linking
  shader->id = glCreateProgram();
  glAttachShader(shader->id, shader->vert_id);
  glAttachShader(shader->id, shader->frag_id);
  glLinkProgram(shader->id);
  check_shader_linker_error(shader);
  
  // Detaching
  glDetachShader(shader->id, shader->vert_id);
  glDetachShader(shader->id, shader->frag_id);
  glDeleteShader(shader->vert_id);
  glDeleteShader(shader->frag_id);

  return shader;
}

void gfx_shader_destroy(GfxShader* shader) {
  if(!shader) {
    return;
  }
 
  memory_free(shader->vert_src);
  memory_free(shader->frag_src);
  glDeleteProgram(shader->id);

  memory_free(shader);
}

void gfx_shader_attach_uniform(GfxContext* gfx, GfxShader* shader, const GfxShaderType type, GfxBuffer* buffer) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");
  NIKOL_ASSERT(shader, "Invalid GfxShader struct passed");

  switch (type) {
    case GFX_SHADER_VERTEX: 
      glBindBufferBase(GL_UNIFORM_BUFFER, shader->vertex_uniforms_count, buffer->id);

      shader->vertex_uniforms[shader->vertex_uniforms_count] = buffer;
      shader->vertex_uniforms_count++;
      break;
    case GFX_SHADER_PIXEL: 
      glBindBufferBase(GL_UNIFORM_BUFFER, shader->fragment_uniforms_count, buffer->id);

      shader->fragment_uniforms[shader->fragment_uniforms_count] = buffer;
      shader->fragment_uniforms_count++;
      break;
    case GFX_SHADER_GEOMETRY: 
      break;
  }
}

i32 gfx_glsl_get_uniform_location(GfxShader* shader, const i8* uniform_name) {
  NIKOL_ASSERT(shader, "Invalid GfxShader struct passed");

  i32 loc = glGetUniformLocation(shader->id, uniform_name);
  
  // Cannot do anything with an invalid uniform name
  if(loc == -1) {
    NIKOL_LOG_WARN("Could not find shader uniform with name \'%s\'", uniform_name); 
  }
  
  return loc;
}

void gfx_glsl_upload_uniform_array(GfxShader* shader, const i32 location, const sizei count, const GfxLayoutType type, const void* data) {
  NIKOL_ASSERT(shader, "Invalid GfxShader struct passed");

  // Will not do anything with an invalid uniform
  if(location == -1) {
    return;
  }

  glUseProgram(shader->id);

  switch(type) {
    case GFX_LAYOUT_FLOAT1:
      glUniform1fv(location, count, (f32*)data);
      break;
    case GFX_LAYOUT_FLOAT2:
      glUniform1fv(location, count, (f32*)data);
      break;
    case GFX_LAYOUT_FLOAT3:
      glUniform1fv(location, count, (f32*)data);
      break;
    case GFX_LAYOUT_FLOAT4:
      glUniform1fv(location, count, (f32*)data);
      break;
    case GFX_LAYOUT_INT1:
      glUniform1iv(location, count, (i32*)data);
      break;
    case GFX_LAYOUT_INT2:
      glUniform1iv(location, count, (i32*)data);
      break;
    case GFX_LAYOUT_INT3:
      glUniform1iv(location, count, (i32*)data);
      break;
    case GFX_LAYOUT_INT4:
      glUniform1iv(location, count, (i32*)data);
      break;
    case GFX_LAYOUT_UINT1:
      glUniform1uiv(location, count, (u32*)data);
      break;
    case GFX_LAYOUT_UINT2:
      glUniform1uiv(location, count, (u32*)data);
      break;
    case GFX_LAYOUT_UINT3:
      glUniform1uiv(location, count, (u32*)data);
      break;
    case GFX_LAYOUT_UINT4:
      glUniform1uiv(location, count, (u32*)data);
      break;
    case GFX_LAYOUT_MAT2:
      glUniformMatrix2fv(location, count, GL_FALSE, (f32*)data);
      break;
    case GFX_LAYOUT_MAT3:
      glUniformMatrix3fv(location, count, GL_FALSE, (f32*)data);
      break;
    case GFX_LAYOUT_MAT4:
      glUniformMatrix4fv(location, count, GL_FALSE, (f32*)data);
      break;
  }
}

void gfx_glsl_upload_uniform(GfxShader* shader, const i32 location, const GfxLayoutType type, const void* data) {
  gfx_glsl_upload_uniform_array(shader, location, 1, type, data);
}

/// Shader functions 
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// Texture functions 

GfxTexture* gfx_texture_create(GfxContext* gfx, const GfxTextureDesc& desc) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");
  
  GfxTexture* texture = (GfxTexture*)memory_allocate(sizeof(GfxTexture));
  memory_zero(texture, sizeof(GfxTexture));
 
  texture->desc = desc;

  GLenum gl_wrap_format = get_texture_wrap(desc.wrap_mode);
 
  GLenum in_format, base_format;
  get_texture_format(desc.format, &in_format, &base_format);

  GLenum min_filter, mag_filter;
  get_texture_filter(desc.filter, &min_filter, &mag_filter); 

  glCreateTextures(GL_TEXTURE_2D, 1, &texture->id);
  gl_check_error("glCreateTextures");

  glBindTexture(GL_TEXTURE_2D, texture->id);
  glTexImage2D(GL_TEXTURE_2D, 
               desc.depth, 
               in_format, 
               desc.width, 
               desc.height, 
               0, 
               base_format, 
               GL_UNSIGNED_BYTE, 
               desc.data);
  gl_check_error("glTexImage2D");
  
  glGenerateTextureMipmap(texture->id);
  gl_check_error("glGenerateTextureMipmap");

  glTextureParameteri(texture->id, GL_TEXTURE_WRAP_S, gl_wrap_format);
  glTextureParameteri(texture->id, GL_TEXTURE_WRAP_T, gl_wrap_format);
  glTextureParameteri(texture->id, GL_TEXTURE_MIN_FILTER, min_filter);
  glTextureParameteri(texture->id, GL_TEXTURE_MAG_FILTER, mag_filter);
  gl_check_error("glTextureParameteri");

  return texture;
}

void gfx_texture_destroy(GfxTexture* texture) {
  if(!texture) {
    return;
  }
  
  glDeleteTextures(1, &texture->id);
  memory_free(texture);
}

void gfx_texture_update(GfxContext* gfx, GfxTexture* texture, const GfxTextureDesc& desc) {
  GLenum gl_wrap_format = get_texture_wrap(desc.wrap_mode);
  
  GLenum in_format, base_format;
  get_texture_format(desc.format, &in_format, &base_format);

  GLenum min_filter, mag_filter;
  get_texture_filter(desc.filter, &min_filter, &mag_filter);

  glTextureSubImage2D(texture->id, 
                      desc.depth,
                      0, 
                      0, 
                      desc.width, 
                      desc.height,
                      in_format,
                      GL_UNSIGNED_BYTE, 
                      desc.data);
  glGenerateTextureMipmap(texture->id);

  glTextureParameteri(texture->id, GL_TEXTURE_WRAP_S, gl_wrap_format);
  glTextureParameteri(texture->id, GL_TEXTURE_WRAP_T, gl_wrap_format);
  glTextureParameteri(texture->id, GL_TEXTURE_MIN_FILTER, min_filter);
  glTextureParameteri(texture->id, GL_TEXTURE_MAG_FILTER, mag_filter);
}

/// Texture functions 
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// Pipeline functions 

GfxPipeline* gfx_pipeline_create(GfxContext* gfx, const GfxPipelineDesc& desc) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");

  GfxPipeline* pipe = (GfxPipeline*)memory_allocate(sizeof(GfxPipeline));
  memory_zero(pipe, sizeof(GfxPipeline));

  pipe->desc = desc;

  // VAO init
  glCreateVertexArrays(1, &pipe->vertex_array);

  // Layout init 
  sizei stride = set_buffer_layout(pipe->vertex_array, desc.layout, desc.layout_count); 

  NIKOL_ASSERT(desc.vertex_buffer, "Must have a vertex buffer to create a GfxPipeline struct");

  // VBO init
  pipe->vertex_buffer = desc.vertex_buffer; 
  pipe->vertex_count  = desc.vertices_count; 

  glVertexArrayVertexBuffer(pipe->vertex_array, 0, pipe->vertex_buffer->id, 0, stride);

  // EBO init
  if(desc.index_buffer) {
    pipe->index_buffer = desc.index_buffer;
    pipe->index_count  = desc.indices_count;

    glVertexArrayElementBuffer(pipe->vertex_array, pipe->index_buffer->id);
  }
  
  // Shader init 
  pipe->shader = desc.shader; 
  
  // Set the draw mode for the whole pipeline
  pipe->draw_mode = desc.draw_mode;

  // Textures init
  pipe->textures_count = desc.textures_count; 
  if(desc.textures_count > 0) {
    for(sizei i = 0; i < desc.textures_count; i++) {
      pipe->textures[i] = desc.textures[i]->id;
    }
  }

  return pipe;
}

void gfx_pipeline_destroy(GfxPipeline* pipeline) {
  NIKOL_ASSERT(pipeline, "Attempting to free an invalid GfxPipeline");

  // Deleting the buffers
  glDeleteVertexArrays(1, &pipeline->vertex_array);

  // Free the pipeline
  memory_free(pipeline);
}

void gfx_pipeline_draw_vertex(GfxContext* gfx, GfxPipeline* pipeline) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");
  NIKOL_ASSERT(pipeline, "Invalid GfxPipeline struct passed");
  NIKOL_ASSERT(pipeline->vertex_buffer, "Must have a valid vertex buffer to draw");

  // Bind the shader
  glUseProgram(pipeline->desc.shader->id);

  // Setting the uniform buffers
  send_vertex_uniform_buffers(pipeline->desc.shader); 
  send_fragment_uniform_buffers(pipeline->desc.shader); 

  // Draw the textures
  if(pipeline->textures_count > 0) {
    glBindTextures(0, pipeline->textures_count, pipeline->textures);
  }

  // Draw the vertices
  GLenum draw_mode = get_draw_mode(pipeline->desc.draw_mode); 
  glDrawArrays(draw_mode, 0, pipeline->desc.vertices_count);

  // Unbind the vertex array for debugging purposes
  glBindVertexArray(0);
}

void gfx_pipeline_draw_index(GfxContext* gfx, GfxPipeline* pipeline) {
  NIKOL_ASSERT(gfx, "Invalid GfxContext struct passed");
  NIKOL_ASSERT(pipeline, "Invalid GfxPipeline struct passed");
  NIKOL_ASSERT(pipeline->vertex_buffer, "Must have a valid vertex buffer to draw");
  NIKOL_ASSERT(pipeline->index_buffer, "Must have a valid index buffer to draw");

  // Bind the shader
  glUseProgram(pipeline->desc.shader->id);

  // Setting the uniform buffers
  send_vertex_uniform_buffers(pipeline->desc.shader); 
  send_fragment_uniform_buffers(pipeline->desc.shader); 

  // Draw the textures
  if(pipeline->textures_count > 0) {
    glBindTextures(0, pipeline->textures_count, pipeline->textures);
  }

  // Draw the indices
  GLenum draw_mode = get_draw_mode(pipeline->desc.draw_mode); 
  glDrawElements(draw_mode, pipeline->desc.indices_count, GL_UNSIGNED_INT, 0);
  
  // Unbind the vertex array for debugging purposes
  glBindVertexArray(0);
}

/// Pipeline functions 
///---------------------------------------------------------------------------------------------------------------------

/// *** Graphics ***
/// ---------------------------------------------------------------------

} // End of nikol

#endif // OpenGL check

//////////////////////////////////////////////////////////////////////////
