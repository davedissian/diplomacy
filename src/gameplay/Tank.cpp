#include "Common.h"
#include "Tank.h"

Tank::Tank(const Vec2& position) : Unit(position), speed_{100.0f}
{
    tank_shape_.setSize({6.0f, 12.0f});
    tank_shape_.setFillColor(sf::Color::White);
}

void Tank::draw(sf::RenderWindow *window, World *world) {
    tank_shape_.setPosition(toSFML(position_));
    window->draw(tank_shape_);
}

float Tank::speed() const {
    return speed_;
}
