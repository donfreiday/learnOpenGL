/* g++ -lglfw -lGLEW -lGLU -lGL src/vertex-buffer-triangle.cpp -o bin/a.out && bin/a.out */

#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

// Compiler-specific macro to break execution on OpenGL errors.
#ifdef _WIN32
#define DEBUG_BREAK __debugbreak()
#else
#include <csignal>
#define DEBUG_BREAK raise(SIGTRAP)
#endif
#define ASSERT(x) if (!x) DEBUG_BREAK;

// This macro won't work for one line if statements, etc
// Wrap GL calls in this to check for errors
#define GLCall(x) GLClearError();\
  x;\
  ASSERT(GLLogCall(#x, __FILE__, __LINE__)); // #x turns x into a string.

static void GLClearError() {
  while (glGetError() != GL_NO_ERROR); // GL_NO_ERROR = 0, so could be while(!glGetError())
}

// Returns true for success, false on error
static bool GLLogCall(const char* function, const char* file, int line) {
  while (GLenum error = glGetError()) {
    std::cout << "OpenGL Error 0x" << std::setfill('0') << std::setw(4)
              << std::hex << error << " : " << std::dec << function << " in "
              << file << ":" << line << std::endl;
    return false;
  }
  return true;
}

struct ShaderProgramSource {
  std::string VertexSource;
  std::string FragmentSource;
};

// Parse shader from file
static ShaderProgramSource ParseShader(const std::string& filepath) {
  std::ifstream stream(filepath);

  enum class ShaderType {
    NONE = -1, VERTEX = 0, FRAGMENT = 1
  };

  std::string line;
  std::stringstream ss[2];
  ShaderType type = ShaderType::NONE;
  while (getline(stream, line)) {
    if (line.find("#shader") != std::string::npos) {
      if (line.find("vertex") != std::string::npos) {
        type = ShaderType::VERTEX;
      } 
      else if (line.find("fragment") != std::string::npos) {
        type = ShaderType::FRAGMENT;
      }
    }
    else {
      ss[(int)type] << line << "\n";
    }
  }
  return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str(); // equivalently &source[0]; source must not be out of scope!

    /* void glShaderSource(	  // Replaces the source code in a shader object
     * GLuint shader,         // the ID of our shader program = id from glCreateShader
     * GLsizei count,         // how many source codes are we specifying = 1
     * const GLchar **string, // double pointer, pointer to the actual pointer = memory address of src variable
     * const GLint *length);  // Specifies an array of string lengths. We want to use the whole length, so = nullptr
     */
    GLCall(glShaderSource(id, 1, &src, nullptr));

    /* glCompileShader compiles the source code strings that have been stored in
     * the shader object specified by shader.
     * The compilation status will be stored as part of the shader object's state. This
     * value will be set to GL_TRUE if the shader was compiled without errors and is
     * ready for use, and GL_FALSE otherwise. It can be queried by calling glGetShader
     * with arguments shader and GL_COMPILE_STATUS. */
   GLCall(glCompileShader(id));

   /* void glGetShaderiv( // Returns a parameter from a shader object
    * GLuint shader,      // The shader object to be queried.
 	  * GLenum pname,       // Object parameter; GL_SHADER_TYPE, GL_DELETE_STATUS, GL_COMPILE_STATUS, GL_INFO_LOG_LENGTH, GL_SHADER_SOURCE_LENGTH.
 	  * GLint *params);     // Returns the requested object parameter.
    * iv is the types that the function needs: int and vector (array ie pointer) */
   int result;
   GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
   if (result == GL_FALSE) {
     int length;
     GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));

     // alloca dynamically allocates stack memory
     char *message = (char *)alloca(length * sizeof(char));
     GLCall(glGetShaderInfoLog(id, length, &length, message));
     std::cout << "Error: "
               << ((type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment")
               << " shader compilation failed: " << message << std::endl;
     GLCall(glDeleteShader(id));
     return 0;
   }

   return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    GLCall(unsigned int program = glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    /* glAttachShader attaches a shader object to a program object. In order to
     * create a complete shader program, there must be a way to specify the list
     * of things that will be linked together. */
    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));

    /* glLinkProgram links the program object specified by program. If any
     * shader objects of type GL_VERTEX_SHADER are attached to program, they
     * will be used to create an executable that will run on the programmable
     * vertex processor. If any shader objects of type GL_GEOMETRY_SHADER are
     * attached to program, they will be used to create an executable that will
     * run on the programmable geometry processor. If any shader objects of type
     * GL_FRAGMENT_SHADER are attached to program, they will be used to create
     * an executable that will run on the programmable fragment processor. */
    GLCall(glLinkProgram(program));

    // Validation will be stored as part of the program state.
    GLCall(glValidateProgram(program));

    /* Delete intermediate shader files since they've now been linked into a
     * program. glDetachShader would delete the shader source code. This makes
     * debugging harder, though it technically should be done. */
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

// GLFW callbacks
void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods);

int main(int argc, char **argv) {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    std::cout << "Error: Failed to initialize GLFW.\n";
    return -1;
  }
  std::cout << "Status: Using GLFW version " << glfwGetVersionString() << std::endl;

  // Minimum OpenGL version
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create the window and its OpenGL context.
  GLFWwindow *window = glfwCreateWindow(640, 480, "Modern OpenGL with GLEW", NULL, NULL);
  if (!window) {
    std::cout << "Error: Failed to create Window or OpenGL context.\n";
    glfwTerminate();
  }

  // The key callback is set per window, like other window related callbacks.
  glfwSetKeyCallback(window, key_callback);

  // To use the OpenGL API, you must have a current OpenGL context.
  glfwMakeContextCurrent(window);

  // A valid OpenGL rendering context must be created BEFORE calling glewInit()!
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cout << "Error: glewInit failed: " << glewGetErrorString(err) << std::endl;
    glfwTerminate();
    return -1;
  }

  std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  GLCall(std::cout << "Status: Using OpenGL version " << glGetString(GL_VERSION) << std::endl);

  // These are the vertices of our triangle
  // x,y, x,y, x,y
  float positions[] = {
      -0.5f, -0.5f, // index 0
       0.5f, -0.5f, // index 1
       0.5f,  0.5f, // index 2
      -0.5f,  0.5f, // index 3
  };

  // Index buffer; each int refers to the index of a vertice.
  unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
  };

  unsigned int buffer;
  GLCall(glGenBuffers(1, &buffer));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer)); 
  GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

  GLCall(glEnableVertexAttribArray(0));
  // Once our buffer is bound, we can tell OpenGL about the attribute layout
  /* void glVertexAttribPointer(
   * GLuint index,            // index = 0, since it is the first attribute
   * GLint size,              // How many components (floats) represent this vertex attribute = 2, since we have an x and y component
   * GLenum type,             // GL_FLOAT since they are floats
   * GLboolean normalized,    // False. If true, integer RGB values (0-255) will be converted by OpenGL to 0.0f - 1.0f format
   * GLsizei stride,          // byte offset between consecutive generic vertex attributes. sizeof(float*2)
   * const GLvoid * pointer); // First and only attribute so zero
   */
  GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0));

  // Index buffer is generated, bound, and populated with data very similar to vertex buffer
  unsigned int ibo;
  GLCall(glGenBuffers(1, &ibo));
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo)); 
  GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

  // Shaders
  ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
  unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
  GLCall(glUseProgram(shader));
  
  while (!glfwWindowShouldClose(window)) {
    /* Clear buffers to preset values, in this case the buffers currently
     * enabled for color writing */
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    // Draw the current bound buffer
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_INT, nullptr)); // Example error

    /* Swap the front and the back buffers */
    glfwSwapBuffers(window);

    /* GLFW needs to communicate regularly with the window system both in order
     * to receive events and to show that the application hasn't locked up. */
    glfwPollEvents();
  }

  // Clean up
  GLCall(glDeleteProgram(shader));
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

void error_callback(int error, const char *description) {
  std::cout<< "Error: GLFW error: " << description << std::endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

