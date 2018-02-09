#include "Common.h"
#include "Squad.h"

Squad::Squad(const Vec2& position) : Unit(position), speed_{50.0f}
{
    shape_.setRadius(10.0f);
    shape_.setFillColor(sf::Color::White);
}

void Squad::draw(sf::RenderWindow *window, World *world) {
    shape_.setPosition(toSFML(position_));
    window->draw(shape_);
}

float Squad::speed() const {
    return speed_;
}
