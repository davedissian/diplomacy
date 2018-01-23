#pragma once

#include "Orders.h"

class World;

class Unit {
public:
    Unit();
    virtual ~Unit();

    void addOrder(UniquePtr<Order> order);

    void stepTowards(float dt, const Vec2& direction, float factor);

    virtual void tick(float dt);
    virtual void draw(sf::RenderWindow* window, World* world) = 0;

    virtual float speed() const = 0;

protected:
    Vec2 position_;
    OrderList orders_;
};