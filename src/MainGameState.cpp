#include "Common.h"
#include "Game.h"
#include "MainGameState.h"

//#define DEBUG_GUI

const int BOUNDARY_SIZE = 100;

MainGameState::MainGameState(Game* game) : GameState(game), camera_movement_speed_{0.0f, 0.0f}, show_orders_{false} {
  const int world_size_preset = 1;
  const int num_players = 8;

  switch (world_size_preset)
  {
    case 0: world_ = make_unique<World>(400, Vec2{ 0.0f, 0.0f }, Vec2{ 1800.0f, 1800.0f }); break;
    case 1: world_ = make_unique<World>(800, Vec2{ 0.0f, 0.0f }, Vec2{ 2400.0f, 2400.0f }); break;
    case 2: world_ = make_unique<World>(1600, Vec2{ 0.0f, 0.0f }, Vec2{ 4800.0f, 2400.0f }); break;
  }

  // Create states and set up players to take ownership of states.
  world_->fillStates(num_players);
  auto states = world_->states();
  for (auto state_pair : states) {
    // Set up units.
    Vector<SharedPtr<Unit>> units;
    units.emplace_back(std::make_shared<Squad>(state_pair.second->midpoint()));
    units_.insert(units_.end(), units.begin(), units.end());

    // Create player.
    WeakPtr<State> state = state_pair.second;
    Vector<WeakPtr<Unit>> units_weak;
    for (auto& ptr : units) {
      units_weak.push_back(WeakPtr<Unit>(ptr));
    }
    players_.emplace_back(std::make_shared<Player>(String("Player ") + std::to_string(state_pair.first), state, units_weak));

    // Name state.
    //state_pair.second->setName(String("State ") + std::to_string(state_pair.first));
  }

  // Set up local controller.
  local_controller_ = make_unique<LocalController>();
  local_controller_->possess(players_[0].get());

  // Set up viewport.
  target_centre_ = {0.0f, 0.0f};
  viewport_.setCenter({0.0f, 0.0f});
  target_size_ = game_->screenSize();
}

MainGameState::~MainGameState() {
}

void MainGameState::tick(float dt) {
  // Update camera.
  target_centre_ += camera_movement_speed_ * dt;
  Vec2 current_centre = fromSFML(viewport_.getCenter());
  viewport_.setCenter(toSFML(damp(current_centre, target_centre_, 0.4f, 0.1f, dt)));
  Vec2 current_size = fromSFML(viewport_.getSize());
  viewport_.setSize(toSFML(damp(current_size, target_size_, 0.4f, 0.1f, dt)));

  // Update units.
  for (auto& unit : units_) {
    unit->tick(dt);
  }
}

void MainGameState::draw(sf::RenderWindow* window) {
  // Draw world.
  world_->draw(window);

  // Print interaction state.
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(400, 150));
  ImGui::Begin("Interaction State");
  ImGui::Text("Pending Mode: %d", (int)interaction_pending_.mode);
  switch (interaction_pending_.mode) {
    case InteractionMode::Unit:
      ImGui::Text("Selected unit: %p", interaction_pending_.selected_unit);
      break;
    case InteractionMode::Site:
      ImGui::Text("Selected tile: %p", interaction_pending_.selected_site);
      break;
    case InteractionMode::State:
      ImGui::Text("Selected state: %p", interaction_pending_.selected_state);
      break;
  }
  ImGui::End();

  // Highlight based on pending state.
  switch (interaction_pending_.mode) {
    case InteractionMode::Unit:
      break;
    case InteractionMode::Site:
      if (interaction_pending_.selected_site) {
        auto& site = *interaction_pending_.selected_site;

        sf::Color selected_color(20, 50, 120, 120);
        sf::Color selected_edge_color(20, 50, 120, 200);
        world_->drawTile(window, site, selected_color);
        world_->drawTileEdge(window, site, selected_edge_color);

        // Draw state.
        if (site.owning_state) {
          site.owning_state->setHighlighted(true);
          site.owning_state->draw(window, world_.get());
          site.owning_state->drawBorders(window, world_.get());
          site.owning_state->setHighlighted(false);
        }

#ifdef DEBUG_GUI
        ImGui::SetNextWindowPos(ImVec2(0, 250));
        ImGui::SetNextWindowSize(ImVec2(700, 250));
        ImGui::Begin("Selected Site");
        ImGui::Text("Centre: %f %f", site.centre.x, site.centre.y);
        ImGui::Text("Unusable? %s", site.usable ? "true" : "false");
        for (int i = 0; i < site.edges.size(); ++i) {
            auto& e = site.edges[i];
            auto& n = e.neighbour;
            float angle1 = (float)site.vertexAngle(e.points.front());
            float angle2 = (float)site.vertexAngle(e.points.back());
            if (n != nullptr) {
                ImGui::Text("- Neighbour %d (edge %.0f %.0f (%.1f) to %.0f %.0f (%.1f) diff: %.1f) - centre %.0f %.0f",
                    i,
                    e.points.front().x, e.points.front().y, angle1,
                    e.points.back().x, e.points.back().y, angle2, angle2 - angle1,
                    n->centre.x, n->centre.y);
            }
            else {
                ImGui::Text("- Neighbour %d (edge %.0f %.0f (%.1f) to %.0f %.0f (%.1f) diff: %.1f) - none",
                    i,
                    e.points.front().x, e.points.front().y, angle1,
                    e.points.back().x, e.points.back().y, angle2, angle2 - angle1);
            }
        }
        ImGui::End();
#endif
      }
      break;
    case InteractionMode::State:
      break;
  }

  // Draw units.
  for (auto& unit : units_) {
    unit->draw(window, world_.get());
  }

  // Draw overlays.
  if (show_orders_) {
    for (auto& unit : units_) {
      unit->drawOrderOverlay(window);
    }
  }
}

void MainGameState::handleKey(float dt, sf::Event::KeyEvent& e, bool pressed) {
  if (e.code == sf::Keyboard::LShift) {
    show_orders_ = pressed;
  }
  if (e.code == sf::Keyboard::LControl || e.code == sf::Keyboard::RControl) {
    interaction_pending_ = InteractionState{InteractionMode::Site};
  } else if (e.code == sf::Keyboard::LAlt || e.code == sf::Keyboard::RAlt) {
    interaction_pending_ = InteractionState{InteractionMode::State};
  } else {
    interaction_pending_ = InteractionState{InteractionMode::Unit};
  }
}

void MainGameState::handleMouseMoved(float dt, sf::Event::MouseMoveEvent &e) {
  Vec2i current_mouse_position = { e.x, e.y };
  Vec2i mouse_movement = current_mouse_position - last_mouse_position_;
  last_mouse_position_ = current_mouse_position;

  // Highlight tile underneath cursor.
  Vec2 proj_mouse_position = game_->mapScreenToWorld(current_mouse_position);
  auto find_nearest_site = [&]() -> Map::Site* {
    float nearest_distance_sq = std::numeric_limits<float>::infinity();
    Map::Site* nearest_site = nullptr;
    for (auto &site : world_->mapSites()) {
      float distance = glm::distance2(proj_mouse_position, site.centre);
      if (distance < nearest_distance_sq) {
        nearest_distance_sq = distance;
        nearest_site = &site;
      }
    }
  };
  switch (interaction_pending_.mode) {
    case InteractionMode::Site:
      interaction_pending_.selected_site = find_nearest_site();
      break;
    case InteractionMode::State:
      {
        auto site = find_nearest_site();
        if (site) {
          interaction_pending_.selected_state = site->owning_state;
        }
      }
      break;
  }

  // If the mouse cursor reaches the boundary of the screen, move in that direction, scaled by distance.
  camera_movement_speed_ = { 0.0f, 0.0f };
  const Vec2 speed = fromSFML(viewport_.getSize()) * 0.5f;
  // Right side.
  if (current_mouse_position.x > game_->screenSize().x - BOUNDARY_SIZE) {
    int distance = game_->screenSize().x - current_mouse_position.x;
    if (distance < 0) {
      distance = 0;
    }
    camera_movement_speed_.x += speed.x;
  }
  // Left side.
  if (current_mouse_position.x < BOUNDARY_SIZE) {
    int distance = current_mouse_position.x;
    if (distance < 0) {
      distance = 0;
    }
    camera_movement_speed_.x -= speed.x;
  }
  // Bottom.
  if (current_mouse_position.y > game_->screenSize().y - BOUNDARY_SIZE) {
    int distance = game_->screenSize().y - current_mouse_position.y;
    if (distance < 0) {
      distance = 0;
    }
    camera_movement_speed_.y += speed.y;
  }
  // Top.
  if (current_mouse_position.y < BOUNDARY_SIZE) {
    int distance = current_mouse_position.y;
    if (distance < 0) {
      distance = 0;
    }
    camera_movement_speed_.y -= speed.y;
  }
}

void MainGameState::handleMouseButton(float dt, sf::Event::MouseButtonEvent &e, MouseButtonState state) {
  if (state == MouseButtonState::Pressed) {
    // Save pending interaction.
    if (e.button == sf::Mouse::Left) {
      interaction_state_ = interaction_pending_;
      interaction_pending_ = InteractionState{};
    } else if (e.button == sf::Mouse::Right) {
      // Do action.
      switch (interaction_state_.mode) {
        case InteractionMode::Unit: {
          interaction_state_.selected_unit->addOrder(
              make_unique<MoveOrder>(game_->mapScreenToWorld(last_mouse_position_)),
              show_orders_);
        } break;
      }
    }
  }
}

void MainGameState::handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent &e) {
  const float dead_zone = 0.1f;
  const float scale = 1.5f;
  float factor;
  if (e.delta > dead_zone) {
    factor = 1.0f / scale;
  }
  else if (e.delta < -dead_zone) {
    factor = scale;
  }
  else {
    return;
  }

  // Scale viewport size.
  Vec2 current_size = fromSFML(viewport_.getSize());
  Vec2 new_size = current_size * factor;

  // Calculate new position. If zooming in, move towards cursor, otherwise don't change.
  Vec2 current_centre = fromSFML(viewport_.getCenter());
  Vec2 new_centre = current_centre;
  if (factor < 1.0f) {
    // Lerp between current_centre and mouse_centre.
    // A factor of 0 means we arrive at mouse_centre. 1 means no change from current position.
    new_centre = lerp(game_->mapScreenToWorld({ e.x, e.y }), current_centre, factor);
  }

  target_size_ = new_size;
  target_centre_ = new_centre;
}