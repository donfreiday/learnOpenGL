# Learn OpenGL

This project is based on the  OpenGL tutorial series [here](https://www.youtube.com/watch?v=OR4fNpBjmq8). The platform is Arch Linux, rather than Windows.

The readme is a companion for the accompanying source files, and as such omits many details.

***

## Setting up the environment

 [GLFW](https://www.glfw.org/index.html) (OpenGL FrameWork) is a lightweight crossplatform library to manage our windows and input and acquire an OpenGL context. From the GLFW [FAQ](https://www.glfw.org/faq.html#11---what-is-glfw): 
>GLFW is a small C library that lets you create and manage windows, OpenGL and OpenGL ES contexts and Vulkan surfaces, enumerate monitors and video modes as well as handle inputs such as keyboard, mouse, joystick, clipboard and time.

Rather than building GLFW from source we're going to use Arch's package.
> "Some package systems, for example NuGet, MacPorts and Arch Linux include relatively up-to-date GLFW packages, but most sadly do not. If you intend to use a GLFW package, please verify that its version fits your needs."


```bash
yay -Syu glfw-x11
```

Now we can use GLFW:

```cpp
#include <GLFW/glfw3.h>
```

***

## 01 - Window and Triangle

This is rendered with legacy OpenGL; see commented source.

#### Compilation
```bash
gcc -lglfw -lGL window-and-triangle.cpp -o bin/a.out && bin/a.out
```

#### Notes 

**Contexts**

An [OpenGL context](https://www.khronos.org/opengl/wiki/OpenGL_Context) represents many things. A context stores all of the state associated with this instance of OpenGL.  Think of a context as an object that holds all of OpenGL; when a context is destroyed, OpenGL is destroyed.  A process can create multiple OpenGL contexts. Each context can represent a separate viewable surface, like a window in an application. Each context has its own set of OpenGL Objects, which are independent of those from other contexts. A context's objects can be shared with other contexts. In order for any OpenGL commands to work, a context must be current; all OpenGL commands affect the state of whichever context is current. The current context is a thread-local variable, so a single process can have several threads, each of which has its own current context. However, a single context cannot be current in multiple threads at the same time.

***

### 02 - Modern OpenGL with GLEW
[GLEW](http://glew.sourceforge.net/), the OpenGL Extension Wrangler Library, provides a header file defining OpenGL functions, symbols and constants. GLEW will determine what GPU driver we're using and dynamically links the required libraries. [glad](https://github.com/Dav1dde/glad) is an alternative library for the same purpose.

```bash
yay -Syu glew
```

#### Compilation

```bash
gcc -lglfw -lGLEW -lGLU -lGL modern-opengl-with-glew.cpp -o bin/a.out && bin/a.out
```

#### Notes
```
/* The include order matters! To use GLEW with GLFW, the GLEW header must be
 * included before the GLFW header. Then, after that, include any other required
 * libraries*/
#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
```

### 03 - Vertex Buffers and Drawing a Triangle
[docs.gl](http://docs.gl) is one of the best OpenGL references out there, and you should read up on any functions used.

#### Notes
 OpenGL is a state machine, and every object created is assigned an integer ID, which is used to reference that resource. 

A **vertex buffer** is a memory buffer in VRAM. A  **shader** is a program that runs on the GPU and tells it how to use the data in the  vertex buffer. We bind states and then issue a **draw call** to have our GPU draw this to the screen.

The **Rendering pipeline** call order, simplified: Draw call ->  Vertex shaders  -> fragment (aka pixel) shaders.

High level overview for **shaders**: vertex shaders determine position of vertices, fragment shaders determine the color of individual pixers.

**Vertex shaders** are called once for each vertex, taking in all vertex attributes specified in our buffer. Shaders can receive data from the CPU (uniforms).

**Fragment shaders** run once for each pixel which needs to be rasterized (drawn to screen) - rasterization is 'filling in' the triangles defined by our vertex shader. It's primary purpose is applying color. They run many more times than vertex shaders, they are 'more expensive' in terms of performance. Data can be passed from vertex shaders to fragment shaders.



A **vertex** is not just a position; it can hold much more data: texture coordinates, normals, colors, binormals, etc.
