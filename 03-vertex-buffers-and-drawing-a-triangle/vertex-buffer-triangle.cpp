/* Todo: use CMake. In the meantime:
 * g++ -lglfw -lGLEW -lGLU -lGL vertex-buffer-triangle.cpp -o bin/a.out && bin/a.out */

/* The include order matters! To use GLEW with GLFW, the GLEW header must be
 * included before the GLFW header. Then, after that, include any other required
 * libraries*/
#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string>

// Called by CreateShader
static unsigned int CompileShader(unsigned int type, const std::string& source) {
    // Creates a shader object of the specified type.
    unsigned int id = glCreateShader(type);

    // OpenGL wants a c-string containing the shader source.
    const char* src = source.c_str(); // equivalently &source[0]; source must not be out of scope!

    /* void glShaderSource(	  // Replaces the source code in a shader object
     * GLuint shader,         // the ID of our shader program = id from glCreateShader
     * GLsizei count,         // how many source codes are we specifying = 1
     * const GLchar **string, // double pointer, pointer to the actual pointer = memory address of src variable
     * const GLint *length);  // Specifies an array of string lengths. We want to use the whole length, so = nullptr
     */
    glShaderSource(id, 1, &src, nullptr);

    /* glCompileShader compiles the source code strings that have been stored in
     * the shader object specified by shader.
     * The compilation status will be stored as part of the shader object's state. This
     * value will be set to GL_TRUE if the shader was compiled without errors and is
     * ready for use, and GL_FALSE otherwise. It can be queried by calling glGetShader
     * with arguments shader and GL_COMPILE_STATUS. */
   glCompileShader(id);

   
   /* void glGetShaderiv( // Returns a parameter from a shader object
    * GLuint shader,      // The shader object to be queried.
 	  * GLenum pname,       // Object parameter; GL_SHADER_TYPE, GL_DELETE_STATUS, GL_COMPILE_STATUS, GL_INFO_LOG_LENGTH, GL_SHADER_SOURCE_LENGTH.
 	  * GLint *params);     // Returns the requested object parameter.
    * iv is the types that the function needs: int and vector (array ie pointer) */
   int result;
   glGetShaderiv(id, GL_COMPILE_STATUS, &result);
   if (result == GL_FALSE) {
     int length;
     glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

     // alloca dynamically allocates stack memory
     char *message = (char*)alloca(length * sizeof(char)); 

     /* void glGetShaderInfoLog( // Returns the information log for a shader object
      * GLuint shader,           // Specifies the shader object
      * GLsizei maxLength,       // size of the character buffer for storing the returned information log.
      * GLsizei *length,         // Returns the length of the string returned in infoLog (excluding the null terminator). 
      * GLchar *infoLog);        //  Specifies an array of characters that is used to return the information log.*/
     glGetShaderInfoLog(id, length, &length, message);
     fprintf(stderr, "Error: %s shader compilation failed: %s\n", (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment", message);
     glDeleteShader(id);
     return 0;
   }

   return id;
}

/* This function will compile the two shaders.
 * Declared static so it doesn't leak into other C++ files or translation units.
 * The strings contain the source code to the shaders. 
 * The integer we're returning is the ID of the shader combination */
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    /* glAttachShader attaches a shader object to a program object. In order to
     * create a complete shader program, there must be a way to specify the list
     * of things that will be linked together. Program objects provide this
     * mechanism. 
     * This is similar to linking in C++. */
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    /* glLinkProgram links the program object specified by program. If any
     * shader objects of type GL_VERTEX_SHADER are attached to program, they
     * will be used to create an executable that will run on the programmable
     * vertex processor. If any shader objects of type GL_GEOMETRY_SHADER are
     * attached to program, they will be used to create an executable that will
     * run on the programmable geometry processor. If any shader objects of type
     * GL_FRAGMENT_SHADER are attached to program, they will be used to create
     * an executable that will run on the programmable fragment processor. */
    glLinkProgram(program);

    // Validation will be stored as part of the program state.
    glValidateProgram(program);

    /* Delete intermediate shader files since they've now been linked into a
     * program. glDetachShader would delete the shader source code. This makes
     * debugging harder, though it technically should be done. */
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

/* In case a GLFW function fails, an error is reported to the GLFW error
 * callback. You can receive these reports with an error callback. This function
 * must have the signature below. */
void error_callback(int error, const char *description);

/* Each window has a large number of callbacks that can be set to receive all
 * the various kinds of events. To receive key press and release events, create
 * a key callback function.*/
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods);

int main(int argc, char **argv) {
  /* Callback functions must be set, so GLFW knows to call them. The function to
   * set the error callback is one of the few GLFW functions that may be called
   * before initialization, which lets you be notified of errors both during and
   * after initialization. */
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    fprintf(stderr, "Error: Failed to initialize GLFW.\n");
    return -1;
  }
  fprintf(stdout, "Status: Using GLFW version %s\n", glfwGetVersionString());

  /* You can require a minimum OpenGL version by setting the
   * GLFW_CONTEXT_VERSION_MAJOR and GLFW_CONTEXT_VERSION_MINOR hints before
   * creation. If the required minimum version is not supported on the machine,
   * context (and window) creation fails. */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  /* The window and its OpenGL context are created with a single call to
   * glfwCreateWindow, which returns a handle to the created combined window and
   * context object.*/
  GLFWwindow *window =
      glfwCreateWindow(640, 480, "Modern OpenGL with GLEW", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Error: Failed to create Window or OpenGL context.\n");
    glfwTerminate();
  }

  // The key callback, like other window related callbacks, are set per-window.
  glfwSetKeyCallback(window, key_callback);

  // Before you can use the OpenGL API, you must have a current OpenGL context.
  glfwMakeContextCurrent(window);

  /* A valid OpenGL rendering context must be created BEFORE calling glewInit()!
   */
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "Error: glewInit failed: %s\n", glewGetErrorString(err));
    glfwTerminate();
    return -1;
  }
  fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
  fprintf(stdout, "Status: Using OpenGL version %s\n", glGetString(GL_VERSION));

  // These are the vertices of our triangle
  // x,y, x,y, x,y
  float positions[6] = {
      -0.5f, -0.5f, 
       0.0f,  0.5f, 
       0.5f, -0.5f
  };

  // This will hold the ID of the generated buffer
  unsigned int buffer;

  // Generate one buffer and give us its ID
  glGenBuffers(1, &buffer);

  // Bind (select) our buffer
  glBindBuffer(GL_ARRAY_BUFFER, buffer);

  // Copy our vertice data into our VRAM buffer
  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

  /* We have to enable each vertix attribute array so it'll be drawn. This can
   * be done before calling glVertexAttribPointer, since OpenGL is a state
   * machine */
  glEnableVertexAttribArray(0);

  // Once our buffer is bound, we can tell OpenGL about the attribute layout
  /* void glVertexAttribPointer(
   * GLuint index,            // index = 0, since it is the first attribute
   * GLint size,              // How many components (floats) represent this vertex attribute = 2, since we have an x and y component
   * GLenum type,             // GL_FLOAT since they are floats
   * GLboolean normalized,    // False. If true, integer RGB values (0-255) will be converted by OpenGL to 0.0f - 1.0f format
   * GLsizei stride,          // byte offset between consecutive generic vertex attributes. sizeof(float*2)
   * const GLvoid * pointer); // First and only attribute so zero
   */
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0);

  // Vertex shader source code:
  std::string vertexShader = 
    "#version 330 core                      \n" // Use GLSL version 330, core means no deprecated functions allowed
    "                                       \n"
    "layout(location = 0) in vec4 position; \n" // Our vertex position we've passed in via attribute pointer 0 above
    "                                       \n"
    "void main() {                          \n"
    "  gl_Position = position;              \n"
    "}                                      \n";

  // Fragment shader source code:
  std::string fragmentShader = 
    "#version 330 core                      \n"
    "                                       \n"
    "layout(location = 0) out vec4 color;   \n" // Our vertex we've passed in via attribute pointer 0 above
    "                                       \n"
    "void main() {                          \n"
    "  color = vec4(1.0, 0.0, 0.0, 1.0);    \n" // RGBA
    "}                                      \n";
  
  // Compile our shader sources into a shader program:
  unsigned int shader = CreateShader(vertexShader, fragmentShader);

  // Bind (select) our shader:
  glUseProgram(shader);
  
  while (!glfwWindowShouldClose(window)) {
    // Render here

    /* Clear buffers to preset values, in this case the buffers currently
     * enabled for color writing */
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the current bound buffer
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* Swap the front and the back buffers */
    glfwSwapBuffers(window);

    /* GLFW needs to communicate regularly with the window system both in order
     * to receive events and to show that the application hasn't locked up.
     * Event processing must be done regularly while you have visible windows
     * and is normally done each frame after buffer swapping. There are two
     * methods for processing pending events; polling and waiting. This example
     * will use event polling, which processes only those events that have
     * already been received and then returns immediately. This is the best
     * choice when rendering continually, like most games do.*/
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glDeleteProgram(shader);

  // This destroys any remaining windows and releases any other resources
  // allocated by GLFW.
  glfwTerminate();

  return 0;
}

void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: GLFW error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

