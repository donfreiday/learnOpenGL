/* Todo: use CMake. In the meantime:
 * g++ -lglfw -lGL window-and-triangle.cpp -o bin/a.out && bin/a.out */

#include <GLFW/glfw3.h>
#include <stdio.h>

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
    fprintf(stderr, "Failed to initialize GLFW.\n");
    return -1;
  }

  /* You can require a minimum OpenGL version by setting the
   * GLFW_CONTEXT_VERSION_MAJOR and GLFW_CONTEXT_VERSION_MINOR hints before
   * creation. If the required minimum version is not supported on the machine,
   * context (and window) creation fails. */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  /* The window and its OpenGL context are created with a single call to
   * glfwCreateWindow, which returns a handle to the created combined window and
   * context object */
  GLFWwindow *window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create Window or OpenGL context.\n");
    glfwTerminate();
  }

  // The key callback, like other window related callbacks, are set per-window.
  glfwSetKeyCallback(window, key_callback);

  // Before you can use the OpenGL API, you must have a current OpenGL context.
  glfwMakeContextCurrent(window);

  while (!glfwWindowShouldClose(window)) {
    // Render here

    /* Clear buffers to preset values, in this case the buffers currently
     * enabled for color writing */
    glClear(GL_COLOR_BUFFER_BIT);

    // Legacy OpenGL, for testing purposes only
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f(0.0f, 0.5f);
    glVertex2f(0.5f, -0.5f);
    glEnd();

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

  // glfwDestroyWindow(window);

  // This destroys any remaining windows and releases any other resources
  // allocated by GLFW.
  glfwTerminate();

  return 0;
}

void error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}