#pragma once

#include "Orders.h"

class World;

class Unit {
public:
    Unit(const Vec2& position);
    virtual ~Unit();

    void addOrder(UniquePtr<Order> order, bool queue);

    void stepTowards(float dt, const Vec2& direction, float factor);

    virtual void tick(float dt);
    virtual void draw(sf::RenderWindow* window, World* world) = 0;
    void drawOrderOverlay(sf::RenderWindow* window);

    virtual float speed() const = 0;

    const Vec2& position() const { return position_; }

protected:
    Vec2 position_;
    OrderList orders_;
};