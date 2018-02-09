#include "Common.h"
#include "Player.h"
#include "world/State.h"
#include "LocalController.h"

void LocalController::tick(float dt)
{
}

void LocalController::possess(Player *player) {
    if (possessed_) {
        possessed_->state().lock()->setHighlighted(false);
    }
    Controller::possess(player);
    possessed_->state().lock()->setHighlighted(true);
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