#include "Common.h"
#include "player/Player.h"
#include "world/State.h"
#include "gameplay/Unit.h"

Player::Player(const String& name, WeakPtr<State> state, Vector<WeakPtr<Unit>> units) : name_{name}, state_{state}, units_(units) {

}

void Player::tick(float dt) {

}

WeakPtr<State> Player::state() const {
    return state_;
}

