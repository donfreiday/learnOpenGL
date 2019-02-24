# Learn OpenGL

This project is based on the  OpenGL tutorial series [here](https://www.youtube.com/watch?v=OR4fNpBjmq8). The platform is Arch Linux, rather than Windows.

The readme is a companion for the accompanying source files, and as such omits many details.

### Setting up the environment

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

### 01 - Window and Triangle

This is rendered with legacy OpenGL; see commented source.

```bash
gcc -lglfw -lGL window-and-triangle.cpp -o bin/a.out && bin/a.out
```

### 02 - Modern OpenGL with GLEW
[GLEW](http://glew.sourceforge.net/), the OpenGL Extension Wrangler Library, provides a header file defining OpenGL functions, symbols and constants. GLEW will determine what GPU driver we're using and dynamically links the required libraries. [glad](https://github.com/Dav1dde/glad) is an alternative library for the same purpose.

```bash
yay -Syu glew
```

Note:
```
/* The include order matters! To use GLEW with GLFW, the GLEW header must be
 * included before the GLFW header. Then, after that, include any other required
 * libraries*/
#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
```

Compile:

```bash
gcc -lglfw -lGLEW -lGLU -lGL modern-opengl-with-glew.cpp -o bin/a.out && bin/a.out
```