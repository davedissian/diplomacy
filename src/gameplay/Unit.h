#pragma once

#include "Orders.h"

class World;
class Player;

class Unit {
public:
    Unit(const Vec2& position);
    virtual ~Unit();

	void setOwner(WeakPtr<Player> owner);

    void addOrder(UniquePtr<Order> order, bool queue);

    void stepTowards(float dt, const Vec2& direction, float factor);

    virtual void tick(float dt);
    virtual void draw(RenderContext& ctx) = 0;
    void drawOrderOverlay(RenderContext& ctx);

    virtual float speed() const = 0;

    const Vec2& position() const { return position_; }

protected:
    Vec2 position_;
    OrderList orders_;

	WeakPtr<Player> owner_;
};