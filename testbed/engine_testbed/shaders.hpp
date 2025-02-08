#pragma once

inline const char* default3d_shader() {
  return 
    "#version 460 core\n"
    "\n"
    "// Layouts\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "layout (location = 2) in vec2 aTextureCoords;\n"
    "\n"
    "// Outputs\n"
    "out VS_OUT {\n"
    "  vec2 tex_coords;\n"
    "} vs_out;\n"
    "\n"
    "layout (std140, binding = 0) uniform Matrices {\n"
    "  mat4 u_view_projection;\n"
    "};\n"
    "uniform mat4 u_model;"
    "\n"
    "void main() {\n"
    "  vs_out.tex_coords = aTextureCoords;\n"
    "\n"
    "  gl_Position = u_view_projection * u_model * vec4(aPos, 1.0);\n"
    "}"
    "\n"
    "#version 460 core\n"
    "\n"
    "// Outputs\n"
    "layout (location = 0) out vec4 frag_color;\n"
    "\n"
    "// Inputs\n"
    "in VS_OUT {\n"
    "  vec2 tex_coords;\n"
    "} fs_in;\n"
    "\n"
    "// Uniforms\n"
    "uniform sampler2D u_texture;\n"
    "\n"
    "void main() {\n"
    "  frag_color = vec4(1.0);//texture(u_texture, fs_in.tex_coords);\n"
    "};\n";
}

inline const char* cubemap_shader_glsl() {
  return
    "#version 460 core\n"
    "\n"
    "// Layouts\n"
    "layout (location = 0) in vec3 aTextureCoords;\n"
    "\n"
    "// Outputs\n"
    "out VS_OUT {\n"
    "  vec3 tex_coords;\n"
    "} vs_out;\n"
    "\n"
    "layout (std140, binding = 0) uniform Matrices {\n"
    "  mat4 u_model;\n"
    "};\n"
    "\n"
    "void main() {\n"
    "  vs_out.tex_coords = aTextureCoords;\n"
    "\n"
    "  gl_Position = vec4(u_model * vec4(aTextureCoords, 1.0)).xyww;\n"
    "}"
    "\n"
    "#version 460 core\n"
    "\n"
    "// Outputs\n"
    "layout (location = 0) out vec4 frag_color;\n"
    "\n"
    "// Inputs\n"
    "in VS_OUT {\n"
    "  vec3 tex_coords;\n"
    "} fs_in;\n"
    "\n"
    "// Uniforms\n"
    "uniform samplerCube u_cubemap;\n"
    "\n"
    "void main() {\n"
    "  frag_color = texture(u_cubemap, fs_in.tex_coords);\n"
    "};\n";
}

inline const char* post_proc_shader_glsl() {
  return
    "#version 460 core\n"
    "\n" 
    "// Layouts\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTextureCoords;\n"
    "\n" 
    "// Outputs\n"
    "out VS_OUT {\n"
    "  vec2 tex_coords;\n"
    "} vs_out;\n"
    "\n" 
    "void main() {\n"
    "  vs_out.tex_coords = aTextureCoords;\n"
    "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}"
    "\n" 
    "#version 460 core\n"
    "\n" 
    "// Outputs\n"
    "layout (location = 0) out vec4 frag_color;\n"
    "\n" 
    "// Inputs\n"
    "in VS_OUT {\n"
    "  vec2 tex_coords;\n"
    "} fs_in;\n"
    "\n" 
    "// Uniforms\n"
    "uniform sampler2D u_texture;\n"
    "uniform int u_effect_index;"
    "\n" 
    "// Functions\n"
    "vec4 greyscale();\n"
    "vec4 inversion();\n"
    "vec3 kernel(float[9] kernel_values);\n"
    "vec4 sharpen();\n"
    "vec4 blur();\n"
    "vec4 edge_detect();\n"
    "vec4 emboss();\n"
    "\n" 
    "void main() {\n"
    "  vec4 effects[] = {\n"
    "    texture(u_texture, fs_in.tex_coords),"
    "    greyscale(), \n"
    "    inversion(), \n"
    "    sharpen(), \n"
    "    blur(), \n"
    "    emboss(), \n"
    "    edge_detect(),\n"
    "  };\n"
    "\n"
    "  frag_color = effects[u_effect_index];\n"
    "};\n"
    "\n" 
    "vec4 greyscale() {\n"
    "  vec4 col = texture(u_texture, fs_in.tex_coords);\n"
    "  float average = (0.2126f * col.r) + (0.7152f * col.g) + (0.0722f * col.b);\n"
    "  return vec4(average, average, average, 1.0);\n"
    "}\n"
    "\n" 
    "vec4 inversion() {\n" 
    "  return vec4(vec3(1.0 - texture(u_texture, fs_in.tex_coords)), 1.0);\n" 
    "}\n"
    "\n" 
    "vec3 kernel(float[9] kernel_values) {\n"
    "  const float offset = 1 / 300.0f;\n"
    "  vec2 offsets[9] = vec2[](\n"
    "  vec2(-offset, offset), // Top-left\n"
    "  vec2(0.0f, offset),    // Top-center\n"
    "  vec2(offset, offset),  // Top-right\n"
    "\n" 
    "  vec2(-offset, 0.0f),   // Center-left\n"
    "  vec2(0.0f, 0.0f),      // Center\n"
    "  vec2(offset, 0.0f),    // Center-right\n"
    "\n" 
    "  vec2(-offset, -offset), // Bottom-left\n"
    "  vec2(0.0f, -offset),    // Bottom-center\n"
    "  vec2(offset, -offset)   // Bottom-right\n"
    "  );\n"
    "\n" 
    "  vec3 sample_tex[9];\n"
    "  for(int i = 0; i < 9; i++) {\n"
    "    sample_tex[i] = vec3(texture(u_texture, fs_in.tex_coords.st + offsets[i]));\n"
    "  }\n"
    "\n" 
    "  vec3 color = vec3(0.0f);\n"
    "  for(int i = 0; i < 9; i++) {\n"
    "    color += sample_tex[i] * kernel_values[i];\n"
    "  }\n"
    "  return color;\n"
    "}\n"
    "\n" 
    "vec4 sharpen() {\n"
    "  float k[9] = float[](\n"
    "    -1.0f, -1.0f, -1.0f, \n"
    "    -1.0f,  9.0f, -1.0f,\n"
    "    -1.0f, -1.0f, -1.0f\n"
    "  );\n"
    "  return vec4(kernel(k), 1.0f);\n"
    "}\n" 
    "vec4 blur() {\n"
    "  const float mul = 16.0f;\n"
    "  float k[9] = float[](\n"
    "    1.0f / mul, 2.0f / mul, 1.0f / mul,\n"
    "    2.0f / mul, 4.0f / mul, 2.0f / mul,\n"
    "    1.0f / mul, 2.0f / mul, 1.0f / mul \n"
    "  );\n"
    "  return vec4(kernel(k), 1.0f);\n"
    "}\n"
    "\n" 
    "vec4 edge_detect() {\n"
    "  float k[9] = float[](\n"
    "    1.0f,  1.0f, 1.0f, \n"
    "    1.0f, -8.0f, 1.0f, \n"
    "    1.0f,  1.0f, 1.0f  \n"
    "  );\n"
    "  return vec4(kernel(k), 1.0f);\n"
    "}\n"
    "vec4 emboss() {\n"
    "  float k[9] = float[](\n"
    "   -2.0f, -1.0f, 0.0f, \n"
    "   -1.0f,  1.0f, 1.0f, \n"
    "    0.0f,  1.0f, 2.0f  \n"
    "  );\n"
    "  return vec4(kernel(k), 1.0f);\n"
    "}\n"
    "\n";
}
