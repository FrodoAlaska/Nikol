#include "nikola/nikola_core.hpp"
#include "nikola/nikola_engine.hpp"

//////////////////////////////////////////////////////////////////////////

namespace nikola { // Start of nikola

/// ----------------------------------------------------------------------
/// Camera consts
const f32 CAMERA_SPEED = 10.0f;
/// Camera consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Camera functions

void camera_default_move_func(Camera& camera) {
  float speed = CAMERA_SPEED * niclock_get_delta_time();

  Vec2 mouse_offset; 
  input_mouse_offset(&mouse_offset.x, &mouse_offset.y);

  camera.yaw   = mouse_offset.x * camera.sensitivity;
  camera.pitch = mouse_offset.y * camera.sensitivity;

  camera.pitch = clamp_float(camera.pitch, -CAMERA_MAX_DEGREES, CAMERA_MAX_DEGREES);
  camera.zoom  = clamp_float(camera.zoom, 1.0f, CAMERA_MAX_ZOOM);

  // Move forward
  if(input_key_down(KEY_UP)) {
    camera.position += camera.front * speed;
  }
  // Move backwards
  else if(input_key_down(KEY_DOWN)) {
    camera.position -= camera.front * speed;
  }
 
  // Move right
  if(input_key_down(KEY_RIGHT)) {
    camera.position += vec3_normalize(vec3_cross(camera.front, camera.up)) * speed;
  }
  // Move left
  else if(input_key_down(KEY_LEFT)) {
    camera.position -= vec3_normalize(vec3_cross(camera.front, camera.up)) * speed;
  }

  // Don't move on the Y-axis if the camera fixed 
  camera.position.y = 0.0f;
}

void camera_create(Camera* cam, const f32 aspect_ratio, const Vec3& pos, const Vec3& target, const CameraMoveFn& move_fn) {
  memory_zero(cam, sizeof(Camera));

  cam->yaw   = -90.0f; 
  cam->pitch = 0.0f; 

  cam->zoom         = 90.0f;
  cam->aspect_ratio = aspect_ratio;

  Vec3 look_dir(pos - target);
  Vec3 up_axis(0.0f, 1.0f, 0.0f);
  Vec3 right_axis(vec3_cross(up_axis, look_dir));

  cam->position = pos;
  cam->up       = vec3_normalize(vec3_cross(look_dir, right_axis));

  cam->direction.x = nikola::cos(cam->yaw    * nikola::DEG2RAD)  * nikola::cos(cam->pitch * nikola::DEG2RAD);
  cam->direction.y = nikola::sin((cam->pitch * nikola::DEG2RAD));
  cam->direction.z = nikola::sin((cam->yaw   * nikola::DEG2RAD)) * nikola::cos(cam->pitch * nikola::DEG2RAD);
  cam->front       = vec3_normalize(cam->direction);

  cam->view            = Mat4(1.0f);
  cam->projection      = Mat4(1.0f);
  cam->view_projection = Mat4(1.0f);

  cam->move_fn = move_fn;
}

void camera_update(Camera& cam) {
  cam.view            = mat4_look_at(cam.position, cam.position + cam.front, cam.up);
  cam.projection      = mat4_perspective((cam.zoom * nikola::DEG2RAD), cam.aspect_ratio, cam.near, cam.far);
  cam.view_projection = cam.projection * cam.view;

  Vec2 mouse_offset; 
  input_mouse_offset(&mouse_offset.x, &mouse_offset.y);

  if(cam.move_fn) {
    cam.move_fn(cam);
  }

  cam.direction.x = nikola::cos(cam.yaw   * DEG2RAD) * nikola::cos(cam.pitch * DEG2RAD);
  cam.direction.y = nikola::sin(cam.pitch * DEG2RAD);
  cam.direction.z = nikola::sin(cam.yaw   * DEG2RAD) * nikola::cos(cam.pitch * DEG2RAD);
  cam.front       = vec3_normalize(cam.direction);
}

/// Camera functions
/// ----------------------------------------------------------------------

} // End of nikola

//////////////////////////////////////////////////////////////////////////
