#include "Common.h"
#include "Squad.h"
#include "RenderContext.h"

Squad::Squad(const Vec2& position) : Unit(position), speed_{50.0f}
{
    shape_.setRadius(5.0f);
	shape_.setFillColor(sf::Color{150, 150, 150});
}

void Squad::draw(RenderContext& ctx) {
    shape_.setPosition(toSFML(position_));
    ctx.window->draw(shape_);
}

float Squad::speed() const {
    return speed_;
}
