// =========================================================================
// Arena FPS - entry point
//
// This file owns the window, the OpenGL context and the game loop. The loop
// uses a fixed timestep so that physics does not depend on the frame rate
// (see PLAN.md, sections 5 and 8).
// =========================================================================

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gl/Camera.h>
#include <gl/Mesh.h>
#include <gl/Shader.h>

namespace {
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr double kFixedStep = 1.0 / 120.0;
constexpr double kMaxFrameTime = 0.25;
constexpr float kMouseSensitivity = 0.1f;

const char *kVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;

void main() {
  vNormal = mat3(transpose(inverse(model))) * aNormal;
  gl_Position = projection * view * model * vec4(aPosition, 1.0);
}
)";

const char *kFragmentShader = R"(
#version 330 core
in vec3 vNormal;

uniform vec3 color;

out vec4 fragmentColor;

void main() {
  vec3 normal = normalize(vNormal);
  vec3 lightDirection = normalize(vec3(0.4, 0.8, 0.6));
  float diffuse = max(dot(normal, lightDirection), 0.0);
  fragmentColor = vec4(color * (0.25 + 0.75 * diffuse), 1.0);
}
)";

// -------------------------------------------------------------------------
void _ErrorCallback(int code, const char *description) {
  std::cerr << "GLFW error " << code << ": " << description << std::endl;
}

// -------------------------------------------------------------------------
// Unit cube centered at the origin. Each face has its own four vertices so
// that the normals stay flat. Winding is counter-clockwise from outside.
void _MakeCube(std::vector<float> &positions, std::vector<float> &normals,
               std::vector<unsigned int> &indices) {
  constexpr float h = 0.5f;
  struct Face {
    glm::vec3 normal;
    glm::vec3 corners[4];
  };

  const Face faces[6] = {
      // +X
      {{1, 0, 0},
       {{h, -h, h}, {h, -h, -h}, {h, h, -h}, {h, h, h}}},
      // -X
      {{-1, 0, 0},
       {{-h, -h, -h}, {-h, -h, h}, {-h, h, h}, {-h, h, -h}}},
      // +Y
      {{0, 1, 0},
       {{-h, h, h}, {h, h, h}, {h, h, -h}, {-h, h, -h}}},
      // -Y
      {{0, -1, 0},
       {{-h, -h, -h}, {h, -h, -h}, {h, -h, h}, {-h, -h, h}}},
      // +Z
      {{0, 0, 1},
       {{-h, -h, h}, {h, -h, h}, {h, h, h}, {-h, h, h}}},
      // -Z
      {{0, 0, -1},
       {{h, -h, -h}, {-h, -h, -h}, {-h, h, -h}, {h, h, -h}}}};

  positions.clear();
  normals.clear();
  indices.clear();
  for (unsigned int face = 0; face < 6; ++face) {
    const unsigned int base = static_cast<unsigned int>(positions.size() / 3);
    for (unsigned int corner = 0; corner < 4; ++corner) {
      const glm::vec3 &p = faces[face].corners[corner];
      positions.push_back(p.x);
      positions.push_back(p.y);
      positions.push_back(p.z);
      normals.push_back(faces[face].normal.x);
      normals.push_back(faces[face].normal.y);
      normals.push_back(faces[face].normal.z);
    } // end for
    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
  } // end for
}
} // namespace

// -------------------------------------------------------------------------
int main() {
  glfwSetErrorCallback(_ErrorCallback);
  if (!glfwInit()) {
    std::cerr << "Could not initialize GLFW" << std::endl;
    return (EXIT_FAILURE);
  } // end if

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

  GLFWwindow *window =
      glfwCreateWindow(kWindowWidth, kWindowHeight, "Arena FPS", nullptr,
                       nullptr);
  if (window == nullptr) {
    std::cerr << "Could not create the OpenGL 3.3 window" << std::endl;
    glfwTerminate();
    return (EXIT_FAILURE);
  } // end if

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (gladLoaderLoadGL() == 0) {
    std::cerr << "Could not load the OpenGL functions" << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return (EXIT_FAILURE);
  } // end if

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.07f, 0.08f, 0.10f, 1.0f);

  // GPU resources. They must be released before the GL context is destroyed.
  gl::Shader shader;
  gl::Mesh cube;

  if (!shader.LoadFromSource(kVertexShader, kFragmentShader)) {
    std::cerr << "Could not build the cube shader" << std::endl;
    shader.Destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return (EXIT_FAILURE);
  } // end if

  std::vector<float> positions, normals;
  std::vector<unsigned int> indices;
  _MakeCube(positions, normals, indices);
  if (!cube.Upload(positions, normals, indices)) {
    std::cerr << "Could not upload the cube mesh" << std::endl;
    cube.Destroy();
    shader.Destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return (EXIT_FAILURE);
  } // end if

  // First-person camera: eye height 1.7 m, looking slightly down at the cube.
  gl::Camera camera(glm::vec3(0.0f, 1.7f, 3.0f), -90.0f, -7.0f);

  // Mouse capture: only while playing. ESC releases it, click recaptures.
  bool cursorCaptured = true;
  bool firstMouseFrame = true;
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

  double lastMouseX = 0.0, lastMouseY = 0.0;
  glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

  double previousTime = glfwGetTime();
  double accumulator = 0.0;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Toggle mouse capture (the game state machine will own this later).
    if (cursorCaptured &&
        glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      cursorCaptured = false;
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else if (!cursorCaptured &&
               glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) ==
                   GLFW_PRESS) {
      cursorCaptured = true;
      firstMouseFrame = true;
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
    } // end if

    // Mouse look runs once per frame, not once per fixed step.
    if (cursorCaptured) {
      double mouseX = 0.0, mouseY = 0.0;
      glfwGetCursorPos(window, &mouseX, &mouseY);
      if (firstMouseFrame) {
        // The cursor position can jump when capture starts; ignore it.
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        firstMouseFrame = false;
      } else {
        const float deltaX = float(mouseX - lastMouseX);
        const float deltaY = float(mouseY - lastMouseY);
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        camera.AddAngles(deltaX * kMouseSensitivity,
                         -deltaY * kMouseSensitivity);
      } // end if
    } // end if

    // Fixed timestep: simulate in constant increments, render whenever we can.
    const double now = glfwGetTime();
    double frameTime = now - previousTime;
    previousTime = now;
    if (frameTime > kMaxFrameTime)
      frameTime = kMaxFrameTime;
    accumulator += frameTime;

    while (accumulator >= kFixedStep) {
      // TODO(hito 3): update player movement with kFixedStep.
      // TODO(hito 4): resolve collisions with kFixedStep.
      // TODO(hito 6): update weapons, projectiles and damage.
      accumulator -= kFixedStep;
    } // end while

    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, (height > 0) ? height : 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -3.0f));
    model = glm::rotate(model, glm::radians(30.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(20.0f),
                        glm::vec3(1.0f, 0.0f, 0.0f));
    shader.Use();
    shader.SetMat4("model", model);
    shader.SetMat4("view", camera.GetViewMatrix());
    shader.SetMat4("projection", camera.GetProjectionMatrix(width, height));
    shader.SetVec3("color", glm::vec3(0.85f, 0.45f, 0.20f));

    cube.Draw();

    glfwSwapBuffers(window);
  } // end while

  cube.Destroy();
  shader.Destroy();
  glfwDestroyWindow(window);
  glfwTerminate();
  return (EXIT_SUCCESS);
}

// eof - main.cpp
