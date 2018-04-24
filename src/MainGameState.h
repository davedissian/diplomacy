#pragma once

#include "GameState.h"

#include "world/Map.h"
#include "world/World.h"
#include "player/Player.h"
#include "gameplay/Squad.h"
#include "player/LocalController.h"

enum class InteractionMode {
  None,
  Unit,
  Site,
  State
};

struct InteractionState {
  InteractionState() : InteractionState(InteractionMode::None) {}
  InteractionState(InteractionMode mode) : mode{mode}, selected_unit{nullptr}, selected_site{nullptr}, selected_state{nullptr} {}

  InteractionMode mode;
  Unit* selected_unit; // Used by InteractionMode::Unit
  Map::Site* selected_site; // Used by InteractionMode::Site
  State* selected_state; // Used by InteractionMode::State
};

class MainGameState : public GameState
{
public:
  MainGameState(Game* game);
  ~MainGameState() override;

  void tick(float dt) override;
  void draw(sf::RenderWindow* window) override;

  void handleKey(float dt, sf::Event::KeyEvent& e, bool pressed) override;
  void handleMouseMoved(float dt, sf::Event::MouseMoveEvent& e) override;
  void handleMouseButton(float dt, sf::Event::MouseButtonEvent& e, MouseButtonState state) override;
  void handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent& e) override;

private:
  Vec2i last_mouse_position_;
  Vec2 camera_movement_speed_;

  Vec2 target_centre_;
  Vec2 target_size_;

  // Interaction.
  InteractionState interaction_state_, interaction_pending_;

  // World.
  UniquePtr<World> world_;

  // Players.
  Vector<SharedPtr<Player>> players_;

  // Local controller.
  UniquePtr<LocalController> local_controller_;

  // Units.
  Vector<SharedPtr<Unit>> units_;
  bool show_orders_;
};