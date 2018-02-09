#pragma once

class State;
class Unit;

class Player : public EnableSharedFromThis<Player> {
public:
    Player(const String& name, WeakPtr<State> state, Vector<WeakPtr<Unit>> units);
    ~Player() = default;

    void tick(float dt);

    WeakPtr<State> state() const;

private:
    String name_;
    WeakPtr<State> state_;
    Vector<WeakPtr<Unit>> units_;
};