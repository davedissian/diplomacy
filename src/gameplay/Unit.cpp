#include "Common.h"
#include "Unit.h"

Unit::Unit(const Vec2& position) : position_{position}, orders_{this} {
}

Unit::~Unit() {
}

void Unit::setOwner(WeakPtr<Player> owner)
{
	owner_ = owner;
}

void Unit::addOrder(UniquePtr<Order> order, bool queue) {
    orders_.add(std::move(order), queue);
}

void Unit::stepTowards(float dt, const Vec2& direction, float factor)
{
    position_ += direction * factor * speed() * dt;
}

void Unit::tick(float dt) {
    orders_.tick(dt);
}

void Unit::draw(RenderContext&) {
}

void Unit::drawOrderOverlay(RenderContext& ctx) {
    orders_.draw(ctx);
}
