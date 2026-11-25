// =========================================================================
// Minimal Dear ImGui HUD: health, speed and crosshair
// =========================================================================

#include <ui/Hud.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <game/World.h>

#include <glm/glm.hpp>

#include <cstdio>

// -------------------------------------------------------------------------
bool ui::Hud::Initialize(GLFWwindow *window, std::string *error) {
  if (error != nullptr)
    error->clear();

  IMGUI_CHECKVERSION();
  if (ImGui::CreateContext() == nullptr) {
    if (error != nullptr)
      *error = "could not create the ImGui context";
    return (false);
  } // end if

  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr; // keep imgui.ini out of the repository
  ImGui::StyleColorsDark();

  if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
    if (error != nullptr)
      *error = "could not initialize the ImGui GLFW backend";
    ImGui::DestroyContext();
    return (false);
  } // end if

  if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
    if (error != nullptr)
      *error = "could not initialize the ImGui OpenGL3 backend";
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return (false);
  } // end if

  this->initialized = true;
  return (true);
}

// -------------------------------------------------------------------------
void ui::Hud::Render(const game::World &world) {
  if (!this->initialized)
    return;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImDrawList *draw = ImGui::GetForegroundDrawList();
  const ImVec2 display = ImGui::GetIO().DisplaySize;

  const ImU32 white = IM_COL32(255, 255, 255, 255);
  const ImU32 black = IM_COL32(0, 0, 0, 180);
  const ImU32 panel = IM_COL32(0, 0, 0, 120);

  char text[128];

  // Health and speed panel, top-left.
  draw->AddRectFilled(ImVec2(10.0f, 10.0f), ImVec2(220.0f, 70.0f), panel,
                      4.0f);

  const ImU32 healthColor =
      (world.playerHealth.current <= 25) ? IM_COL32(255, 80, 80, 255) : white;
  std::snprintf(text, sizeof(text), "HP  %d / %d", world.playerHealth.current,
                world.playerHealth.maximum);
  draw->AddText(ImVec2(20.0f, 18.0f), healthColor, text);

  const float speed = glm::length(
      glm::vec2(world.player.velocity.x, world.player.velocity.z));
  std::snprintf(text, sizeof(text), "Speed  %.1f m/s", speed);
  draw->AddText(ImVec2(20.0f, 40.0f), white, text);

  // Crosshair, screen center, with a dark outline for readability.
  const ImVec2 center(display.x * 0.5f, display.y * 0.5f);
  constexpr float kHalfLength = 8.0f;

  draw->AddLine(ImVec2(center.x - kHalfLength, center.y),
                ImVec2(center.x + kHalfLength, center.y), black, 3.0f);
  draw->AddLine(ImVec2(center.x, center.y - kHalfLength),
                ImVec2(center.x, center.y + kHalfLength), black, 3.0f);

  draw->AddLine(ImVec2(center.x - kHalfLength, center.y),
                ImVec2(center.x + kHalfLength, center.y), white, 1.0f);
  draw->AddLine(ImVec2(center.x, center.y - kHalfLength),
                ImVec2(center.x, center.y + kHalfLength), white, 1.0f);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// -------------------------------------------------------------------------
void ui::Hud::Shutdown() {
  if (!this->initialized)
    return;

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  this->initialized = false;
}

// eof - Hud.cpp
