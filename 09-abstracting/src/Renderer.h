#pragma once

#include <GL/glew.h>

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

// Clear OpenGL errors
void GLClearError(); 

/******************************************************************************/
// Return false if there are any OpenGl errors.
bool GLLogCall(const char* function, const char* file, int line);