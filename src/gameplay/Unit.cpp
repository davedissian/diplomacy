#include "Common.h"
#include "Unit.h"

Unit::Unit() : orders_{this} {
}

Unit::~Unit() {
}

void Unit::addOrder(UniquePtr<Order> order) {
    orders_.enqueue(std::move(order));
}

void Unit::tick(float dt) {
    orders_.tick(dt);
}

void Unit::draw(sf::RenderWindow *window, World *world) {
}
