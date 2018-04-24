#pragma once

class Game;

class GameState
{
public:
  GameState(Game* game);
  virtual ~GameState() = default;

  virtual void tick(float dt) {}
  virtual void draw(sf::RenderWindow* window) {}

  virtual void handleKey(float dt, sf::Event::KeyEvent& e, bool pressed) {}
  virtual void handleMouseMoved(float dt, sf::Event::MouseMoveEvent& e) {}
  virtual void handleMouseButton(float dt, sf::Event::MouseButtonEvent& e, MouseButtonState state) {}
  virtual void handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent& e) {}

  virtual sf::View& viewport() { return viewport_; }
  virtual const sf::View& viewport() const { return viewport_; }

protected:
  Game* game_;
  sf::View viewport_;
};