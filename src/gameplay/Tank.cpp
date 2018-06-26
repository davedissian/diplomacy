#include "Common.h"
#include "RenderContext.h"
#include "Tank.h"

Tank::Tank(const Vec2& position) : Unit(position), speed_{100.0f}
{
    tank_shape_.setSize({6.0f, 12.0f});
    tank_shape_.setFillColor(sf::Color::White);
}

void Tank::draw(RenderContext& ctx) {
    tank_shape_.setPosition(toSFML(position_));
    ctx.window->draw(tank_shape_);
}

float Tank::speed() const {
    return speed_;
}
