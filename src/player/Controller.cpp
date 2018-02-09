#include "Common.h"
#include "Controller.h"

Controller::Controller() : possessed_{nullptr} {
}

void Controller::tick(float dt) {
}

void Controller::possess(Player *player) {
    possessed_ = player;
}
