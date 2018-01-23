#pragma once

#include "Unit.h"

class Tank : public Unit {
public:
    ~Tank() override = default;

    // Unit
    virtual void draw(sf::RenderWindow* window, World* world) override;
    virtual float speed() const override;

private:
    float speed_;
};