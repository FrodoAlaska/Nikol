# Nikol
A cross-platform framework for window creation, input handling, and rendering using OpenGL or DirectX11. 

# Features 
- Cross-platform window creation. 
- Gamepad, keyboard, and mouse input support.
- Fully-configurable cross-platform rendering API using OpenGL (Linux) and DirectX11 (Windows).

# Dependencies
- GLFW3 
- GLAD
- D3D11

# Build Instructions
// @TODO

# Hello, Nikol
Here's a simple example of the library. The example below will open a basic window and initialze a graphics context.

```c++
#include <nikol_core.hpp>

// Will choose the appropriate main entry based on the platform
int main() {
  // Initialze the library
  if(!nikol::init()) {
    return -1;
  }

  // Openinig the window
  nikol::i32 win_flags = nikol::WINDOW_FLAGS_FOCUS_ON_CREATE | nikol::WINDOW_FLAGS_GFX_HARDWARE;
  nikol::Window* window = nikol::window_open("Hello, Nikol", 1366, 768, win_flags);
  if(!window) {
    return -1;
  }

  // Creating a graphics context
  nikol::GfxContextDesc gfx_desc = {
    .window = window,
    .states = nikol::GFX_STATE_DEPTH | nikol::GFX_STATE_STENCIL,
  };
  nikol::GfxContext* gfx = nikol::gfx_context_init(gfx_desc);
  if(!gfx) {
    return -1;
  }

  // Main loop
  while(nikol::window_is_open(window)) {
    // Will stop the application when F1 is pressed
    if(nikol::input_key_pressed(nikol::KEY_F1)) {
      break;
    }
    
    // Clear the screen to black
    nikol::gfx_context_clear(gfx, 0.0f, 0.0f, 0.0f, 1.0f);
    
    // Swap the internal window buffer
    nikol::gfx_context_present(gfx);
    
    // Poll the window events
    nikol::window_poll_events(window);
  }

  // De-initialze
  nikol::gfx_context_shutdown(gfx);
  nikol::window_close(window);
  nikol::shutdown();
}

```

# More Examples 
For more practical examples using the _Nikol_ engine, go to [NikolExamples]("https://github.com/FrodoAlaska/NikolExamples")
