# *Nikola*

A cross-platform framework for window creation, input handling, audio playback, and rendering using OpenGL 4.5+.

# Dependencies

- Engine Dependencies:
    - [GLFW3](https://github.com/glfw/glfw)
    - [GLAD](https://github.com/Dav1dde/glad)
    - [GLM](https://github.com/g-truc/glm)

- NBR Tool Dependencies:
    - [STB](https://github.com/nothings/stb)
    - [ASSIMP](https://github.com/assimp/assimp)
 
# Features 
- A fully documented single header file for every functionality in the library.
- Cross-platform window creation. 
- Gamepad, keyboard, and mouse input support.
- Fully-configurable cross-platform rendering API using OpenGL 4.5+.
- Robust resource manager with a custom resource format (NBR).
- Support for loading multiple image formats such as jpeg, png, bmp, tga, and way more. 
- Full support for loading 3D models using the following formats: OBJ, FBX, and GLTF.

# Missing Features

Some missing features are not currently in the works (Long-term) while others are planned for the future (Short-term). Some other features may _never_ be implemented since it does not fit the current philosophy of the engine (Never).

- 3D animations (Long-term)
- Audio (Short-term)
- Physcis (Short-term)
- Lightweight Editor (Short-term)
- Scripting (Never)

# Build Instructions

Before proceeding with any build or compilation step, *Nikola* needs to be cloned from the Git repo. 

```bash
git clone --recursive https://github.com/FrodoAlaska/Nikola.git
```

Make sure to add the `--recursive` flag since *Nikola* uses submodules to manage its dependencies. 

And now that *Nikola* is cloned, we can start the build process. 

There are two main ways to build *Nikola*. The first is to use the traditional _CMake_ commands below:

```bash
mkdir build 
cd build 
cmake .. 
```

And then to build *Nikola* you can use:

```bash
cmake --build .
```

Now the second way to build *Nikola* is to use the build scripts found in the `scripts` directory. There are a few build scripts but there are only two important ones: `build-nikola.sh` for Linux and `build-nikola.ps1` for Windows. Each script takes in a few flags to make both the development and build process easier. 

```
   --clean          = Have a new fresh build"              
   --debug          = Build for the debug configuration"   
   --rel            = Build for the release configuration" 
   --jobs [threads] = Threads to use when building"        
   --run-testbed    = Run the testbed examples"            
   --reload-res     = Reload the resources cache"          
   --help           = Display this help message"           
```

The `--run-testbed` and `--reload-res` flags call other scripts in the same directory. Hence, the multiple shell scripts. But, generally, the `build-nikola.*` script is _the_ main and most important build script.

The `--debug` and `--rel` _must_ be used independently. Moreover, one of the flags _have_ to be used to know which configuration to build for. If none of them are used, the script will default to a _Debug_ build. Depending on which flag is passed to the build script, a `build-debug` and/or `build-release` will be created in the main directory. Any build artifacts generated by `--run-testbed` or `--reload-res` will be dumped into one of the previously mentioned directories. 

The `--reload-res` flag will call the `reload-resources.*` script to convert any resources to the `.nbr` engine format for resources. However, the `reload-resources.*` in particular is _very_ specific to the current development environment. You can use your own paths and specific resources in the script or use the `NBR` tool directly. 


# Hello, *Nikola*
Here's a simple example of the _core_ library working in action. The example below will open a basic window and initialze a graphics context.

```c++
#include <nikola/nikola_core.hpp>

int main() {
  // Initialze the library
  if(!nikola::init()) {
    return -1;
  }

  // Openinig the window
  nikola::i32 win_flags = nikola::WINDOW_FLAGS_FOCUS_ON_CREATE | nikola::WINDOW_FLAGS_GFX_HARDWARE;
  nikola::Window* window = nikola::window_open("Hello, Nikola", 1366, 768, win_flags);
  if(!window) {
    return -1;
  }

  // Creating a graphics context
  nikola::GfxContextDesc gfx_desc = {
    .window = window,
    .states = nikola::GFX_STATE_DEPTH | nikola::GFX_STATE_STENCIL,
  };
  nikola::GfxContext* gfx = nikola::gfx_context_init(gfx_desc);
  if(!gfx) {
    return -1;
  }

  // Main loop
  while(nikola::window_is_open(window)) {
    // Will stop the application when F1 is pressed
    if(nikola::input_key_pressed(nikola::KEY_F1)) {
      break;
    }
    
    // Clear the screen to black
    nikola::gfx_context_clear(gfx, 0.0f, 0.0f, 0.0f, 1.0f, nikola::GFX_CONTEXT_FLAGS_ENABLE_VSYNC);
    
    // Swap the internal window buffer
    nikola::gfx_context_present(gfx);
    
    // Poll the window events
    nikola::window_poll_events(window);
  }

  // De-initialze
  nikola::gfx_context_shutdown(gfx);
  nikola::window_close(window);
  nikola::shutdown();
}

```
