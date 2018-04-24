#include "Common.h"
#include "Game.h"
#include "MenuGameState.h"

Game::Game() {}

int Game::run(Vec2i window_size) {

  // Deal with retina display.
#ifdef __APPLE__
  ImGui::GetIO().DisplayFramebufferScale = ImVec2(2.0f, 2.0f);
    window_size *= 2;
#endif

  screen_size_ = window_size;
  window_ = make_unique<sf::RenderWindow>(sf::VideoMode((u32)window_size.x, (u32)window_size.y), "Diplomacy RTS");
  window_->setVerticalSyncEnabled(true);
  ImGui::SFML::Init(*window_);

  current_state_ = make_unique<MenuGameState>(this);

  sf::Clock delta_clock;
  sf::Time dt_time;
  float dt = 1.0f / 60.0f;
  while (window_->isOpen()) {
    sf::Event event;
    while (window_->pollEvent(event)) {
      ImGui::SFML::ProcessEvent(event);
      if (event.type == sf::Event::KeyPressed) {
        current_state_->handleKey(dt, event.key, true);
      }
      if (event.type == sf::Event::KeyReleased) {
        current_state_->handleKey(dt, event.key, false);
      }
      if (event.type == sf::Event::MouseMoved) {
        current_state_->handleMouseMoved(dt, event.mouseMove);
      }
      if (event.type == sf::Event::MouseButtonPressed) {
        current_state_->handleMouseButton(dt, event.mouseButton, MouseButtonState::Pressed);
      }
      if (event.type == sf::Event::MouseButtonReleased) {
        current_state_->handleMouseButton(dt, event.mouseButton, MouseButtonState::Released);
      }
      if (event.type == sf::Event::MouseWheelScrolled) {
        current_state_->handleMouseScroll(dt, event.mouseWheelScroll);
      }
      if (event.type == sf::Event::Closed) {
        window_->close();
      }
    }
    ImGui::SFML::Update(*window_, dt_time);

    // Update.
    current_state_->tick(dt);

    // Draw.
    window_->setView(current_state_->viewport());
    window_->clear(sf::Color::Black);
    current_state_->draw(window_.get());
    ImGui::SFML::Render(*window_);
    window_->display();

    // Update frame timer.
    dt_time = delta_clock.restart();
    dt = dt_time.asSeconds();
  }

  return 0;
}

Vec2 Game::mapScreenToWorld(const Vec2i &screen_position) const {
  return fromSFML(window_->mapPixelToCoords({screen_position.x, screen_position.y}, current_state_->viewport()));
}