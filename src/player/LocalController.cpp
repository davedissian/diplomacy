#include "Common.h"
#include "Player.h"
#include "world/State.h"
#include "LocalController.h"

void LocalController::tick(float dt)
{
}

void LocalController::possess(Player *player) {
    Controller::possess(player);
}

void LocalController::handleKey(float dt, sf::Event::KeyEvent &e, bool pressed)
{
}

void LocalController::handleMouseMoved(float dt, sf::Event::MouseMoveEvent &e)
{
}

void LocalController::handleMouseButton(float dt, sf::Event::MouseButtonEvent &e, MouseButtonState state)
{
}

void LocalController::handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent &e)
{
}