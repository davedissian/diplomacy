#pragma once

#include "Unit.h"

class Squad : public Unit {
public:
    Squad(const Vec2& position);
    ~Squad() override = default;

    // Unit
    virtual void draw(RenderContext& ctx) override;
    virtual float speed() const override;

private:
    float speed_;
    sf::CircleShape shape_;
};