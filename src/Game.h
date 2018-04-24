#pragma once

#include "GameState.h"

class Game {
public:
  Game();

  int run(Vec2i window_size);

  Vec2 mapScreenToWorld(const Vec2i& screen_position) const;

  const Vec2i& screenSize() const {
    return screen_size_;
  }

  template <typename S>
  void switchTo() {
    current_state_ = make_unique<S>(this);
  }

  void exit() {
    window_->close();
  }

private:
  Vec2i screen_size_;
  UniquePtr<sf::RenderWindow> window_;

  UniquePtr<GameState> current_state_;
};