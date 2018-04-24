#include "Common.h"
#include "Game.h"
#include "MainGameState.h"
#include "MenuGameState.h"

MenuGameState::MenuGameState(Game* game) : GameState(game) {
}

MenuGameState::~MenuGameState() {
}

void MenuGameState::tick(float dt) {
  auto window_size = ImVec2(900, 300);
  ImGui::SetNextWindowPos({(game_->screenSize().x - window_size.x) / 2, (game_->screenSize().y - window_size.y) / 2});
  ImGui::SetNextWindowSize(window_size);
  ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
  ImGui::Begin("", 0, flags);
  ImGui::Text("DIPLOMACY");
  if (ImGui::Button("Begin", {150.0f, 25.0f})) {
    game_->switchTo<MainGameState>();
  }
  ImGui::Button("Flag Editor", {150.0f, 25.0f});
  if (ImGui::Button("Exit", {150.0f, 25.0f})) {
    game_->exit();
  }
  ImGui::End();
}