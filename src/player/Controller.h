#pragma once

class Player;

class Controller {
public:
    Controller();
    ~Controller() = default;

    virtual void tick(float dt);

    virtual void possess(Player* player);

protected:
    Player* possessed_;

};
