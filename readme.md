# Learn OpenGL

Following OpenGL tutorial series [here](https://www.youtube.com/watch?v=OR4fNpBjmq8)

### Setting up the environment

 [GLFW](https://www.glfw.org/index.html) (OpenGL FrameWork) is a lightweight crossplatform library to manage our windows and input and acquire an OpenGL context. From the GLFW [FAQ](https://www.glfw.org/faq.html#11---what-is-glfw): 
>GLFW is a small C library that lets you create and manage windows, OpenGL and OpenGL ES contexts and Vulkan surfaces, enumerate monitors and video modes as well as handle inputs such as keyboard, mouse, joystick, clipboard and time.

Rather than building GLFW from source I'm going to use Arch's package.
> "Some package systems, for example NuGet, MacPorts and Arch Linux include relatively up-to-date GLFW packages, but most sadly do not. If you intend to use a GLFW package, please verify that its version fits your needs."


```bash
yay -Syu glfw-x11
```

Now we can use GLFW (see 01-create-window-and-triangle):

```cpp
#include <GLFW/glfw3.h>
```

### 01 - Window and Triangle

```bash
gcc -lglfw -lGL window-and-triangle.cpp -o bin/a.out && bin/a.out
```