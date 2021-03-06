/* g++ -g -lglfw -lGLEW -lGLU -lGL src/vertex-buffer-triangle.cpp -o bin/a.out && bin/a.out */

#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

/******************************************************************************/
// Macro to trigger debug break
#ifdef _WIN32
#define DEBUG_BREAK __debugbreak()
#else
#include <csignal>
#define DEBUG_BREAK raise(SIGTRAP)
#endif
#define ASSERT(x) if (!(x)) DEBUG_BREAK;

/******************************************************************************/
// Wrap GL calls in this to check for errors
// This macro won't work for one line if statements, etc
#define GLCall(x) GLClearError();\
  x;\
  ASSERT(GLLogCall(#x, __FILE__, __LINE__)); // #x turns x into a string.

static void GLClearError() {
  while (glGetError() != GL_NO_ERROR); // GL_NO_ERROR = 0, so could be while(!glGetError())
}

/******************************************************************************/
// Return false if there are any OpenGl errors.
static bool GLLogCall(const char* function, const char* file, int line) {
  while (GLenum error = glGetError()) {
    std::cout << "OpenGL Error 0x" << std::setfill('0') << std::setw(4)
              << std::hex << error << " : " << std::dec << function << " in "
              << file << ":" << line << std::endl;
    return false;
  }
  return true;
}

/******************************************************************************/
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

/******************************************************************************/
// Compile shader and return its ID
static unsigned int CompileShader(unsigned int type, const std::string& source) {
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str(); // equivalently &source[0]; source must not be out of scope!
    
    // Give shader the address of our source code string
    GLCall(glShaderSource(id, 1, &src, nullptr));

    // Compile the source
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    
    // Handle errors
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

/******************************************************************************/
// Combine vertex and fragment shaders into a shader program, returns program ID
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    // Compile our vertex and fragment shaders
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(unsigned int program = glCreateProgram());

    // Tell OpenGL we will be linking our shaders to our new shader program
    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));

    // Perform linking
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

/******************************************************************************/
// GLFW callback prototypes
void error_callback(int error, const char *description);
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods);
/******************************************************************************/

int main(int argc, char **argv) {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    std::cout << "Error: Failed to initialize GLFW.\n";
    return -1;
  }
  std::cout << "Status: Using GLFW version " << glfwGetVersionString() << std::endl;

  // Minimum OpenGL version
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

  // Vsync
  glfwSwapInterval(1);

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
  // This allows reuse of vertex data
  unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
  };

  // Create vertex buffer and copy our vertices into it
  unsigned int buffer;
  GLCall(glGenBuffers(1, &buffer));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer)); 
  GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

  /* Create a vertex array object, since the OpenGL core profile doesn't create
   * one at index 0 by default like the OpenGL compat profile does. */
  unsigned int vao;
  GLCall(glGenVertexArrays(1, &vao));
  GLCall(glBindVertexArray(vao));
  
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
  GLCall(glUseProgram(shader)); // Installs a program object as part of current rendering state

  // Uniforms allow the CPU to send data to shaders on the GPU
  GLCall(int location = glGetUniformLocation(shader, "u_Color"));
  ASSERT(location != -1);
  GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));

  // Unbind everything
  GLCall(glBindVertexArray(0)); 
  GLCall(glUseProgram(0));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); 

  float r = 0.0f;
  float increment = 0.05f;
  
  while (!glfwWindowShouldClose(window)) {
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    GLCall(glUseProgram(shader));
    GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

    GLCall(glBindVertexArray(vao));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    if (r > 1.0f)
      increment = -0.05f;
    else if (r < 0.0f)
      increment = 0.05f;

    r += increment;

    glfwSwapBuffers(window);
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

