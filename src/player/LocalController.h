#pragma once

#include "Controller.h"

class LocalController : public Controller {
public:
    void tick(float dt) override;

    void possess(Player* player) override;

    void handleKey(float dt, sf::Event::KeyEvent& e, bool pressed);
    void handleMouseMoved(float dt, sf::Event::MouseMoveEvent& e);
    void handleMouseButton(float dt, sf::Event::MouseButtonEvent& e, MouseButtonState state);
    void handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent& e);
};