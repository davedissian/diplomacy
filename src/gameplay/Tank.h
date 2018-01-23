#pragma once

#include "Unit.h"

class Tank : public Unit {
public:
    Tank(const Vec2& position);
    ~Tank() override = default;

    // Unit
    virtual void draw(sf::RenderWindow* window, World* world) override;
    virtual float speed() const override;

private:
    float speed_;

    sf::RectangleShape tank_shape_;
};