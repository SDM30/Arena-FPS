// =========================================================================
// Arena FPS - entry point
//
// This file owns the window, the OpenGL context, input and the game loop. The
// loop uses a fixed timestep so that physics does not depend on the frame rate
// (see PLAN.md, sections 5 and 8).
// =========================================================================

#include <cmath>
#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <glm/glm.hpp>

#include <game/World.h>
#include <gl/Camera.h>
#include <level/Level.h>
#include <render/SceneRenderer.h>

namespace {
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr double kFixedStep = 1.0 / 120.0;
constexpr double kMaxFrameTime = 0.25;
constexpr float kMouseSensitivity = 0.1f;
constexpr float kEyeHeight = 1.7f;

glm::vec3 _BuildWishDir(const gl::Camera &camera, float forwardInput,
                        float rightInput) {
  glm::vec3 wishDir(0.0f);
  const float yaw = glm::radians(camera.GetYaw());
  // Rotate the base forward and right vectors based on yaw change
  const glm::vec3 forward(std::cos(yaw), 0.0f, std::sin(yaw));
  const glm::vec3 right(-std::sin(yaw), 0.0f, std::cos(yaw));

  // Compute wishdir
  wishDir = forward * forwardInput + right * rightInput;
  // wishdir * wishdir = same as pythagoras theorem
  const float lengthSq = glm::dot(wishDir, wishDir);
  if (lengthSq > 1e-8f)
    wishDir /= std::sqrt(lengthSq);
  else
    wishDir = glm::vec3(0.0f);

  return wishDir;
}

// -------------------------------------------------------------------------
void _ErrorCallback(int code, const char *description) {
  std::cerr << "GLFW error " << code << ": " << description << std::endl;
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

  GLFWwindow *window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                        "Arena FPS", nullptr, nullptr);
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

  // Test level
  level::Level devlevel;
  std::string error;
  if (!devlevel.Load("levels/dev.lvl", &error)) {
    std::cerr << "Could not load the level: " << error << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return (EXIT_FAILURE);
  }

  // Runtime state derived from the level data. The level itself stays
  // immutable, so restarting only requires calling Initialize again.
  game::World world;
  world.Initialize(devlevel);

  // First-person camera: the level stores the feet position, the camera sits
  // at eye height above it.
  gl::Camera camera(world.PlayerFeet() + glm::vec3(0.0f, kEyeHeight, 0.0f),
                    -90.0f, 0.0f);

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

  bool showCollision = false;
  bool showCollisionKeyDown = false;
  bool autoBhopKeyDown = false;
  bool jumpWasDown = false;
  bool pendingJumpPressed = false;

  bool rendererReady = false;
  {
    // GPU resources live for this scope and are released while the GL context
    // is still current.
    render::SceneRenderer renderer;
    std::string renderError;
    if (!renderer.Initialize(devlevel, &renderError)) {
      error = renderError;
    } else {
      rendererReady = true;

      while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glm::vec3 wishDir(0.0f);
        bool jumpHeld = false;

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

          // Movement inputs
          const float forwardInput =     
            (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? 1.0f : 0.0f) -
            (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? 1.0f : 0.0f);

          const float rightInput =     
            (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? 1.0f : 0.0f) -
            (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? 1.0f : 0.0f);

          wishDir = _BuildWishDir(camera, forwardInput, rightInput);

          jumpHeld = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
          if (jumpHeld && !jumpWasDown)
            pendingJumpPressed = true;
          jumpWasDown = jumpHeld;

        } // end if

        // Debug renders
        const bool down = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
        if (down && !showCollisionKeyDown)
          showCollision = !showCollision;
        showCollisionKeyDown = down;

        // Toggle auto-bhop (held jump) vs normal jump (fresh press).
        const bool autoBhopDown =
            glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS;
        if (autoBhopDown && !autoBhopKeyDown)
          world.movement.Config.AutoBhop = !world.movement.Config.AutoBhop;
        autoBhopKeyDown = autoBhopDown;

        // Fixed timestep: simulate in constant increments, render whenever we
        // can.
        const double now = glfwGetTime();
        double frameTime = now - previousTime;
        previousTime = now;
        if (frameTime > kMaxFrameTime)
          frameTime = kMaxFrameTime;
        accumulator += frameTime;

        while (accumulator >= kFixedStep) {
          // TODO(6): update weapons, projectiles and damage.
          game::Input input;
          input.WishDir = wishDir;
          input.JumpHeld = jumpHeld;
          input.JumpPressed = pendingJumpPressed;
          pendingJumpPressed = false;

          world.Update(static_cast<float>(kFixedStep), input);
          accumulator -= kFixedStep;
        } // end while

        // The simulation owns the feet position; the camera only reads it.
        camera.SetPosition(world.PlayerFeet() +
                           glm::vec3(0.0f, kEyeHeight, 0.0f));

        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);

        render::Options options;
        options.showCollision = showCollision;
        renderer.Draw(camera, devlevel, world, width, height, options);

        char title[128];
        const glm::vec3 feet = world.PlayerFeet();
        std::snprintf(title, sizeof(title),
                      "Arena FPS | pos=(%.2f, %.2f, %.2f) | vy=%.2f | %s | jump=%s",
                      feet.x, feet.y, feet.z, world.player.Velocity.y,
                      world.player.OnGround ? "grounded" : "airborne",
                      world.movement.Config.AutoBhop ? "auto" : "normal");
        glfwSetWindowTitle(window, title);

        glfwSwapBuffers(window);
      } // end while
    } // end if
  }   // renderer destroyed here, before the context

  glfwDestroyWindow(window);
  glfwTerminate();

  if (!rendererReady) {
    std::cerr << "Could not initialize the renderer: " << error << std::endl;
    return (EXIT_FAILURE);
  } // end if

  return (EXIT_SUCCESS);
}

// eof - main.cpp
