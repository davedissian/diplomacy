#pragma once

class Unit;

class Order {
public:
    Order();
    virtual ~Order() = default;

    // Draw when visible.
    virtual void draw(sf::RenderWindow* window) = 0;

    // True: completed. False: Requires more processing.
    virtual bool tick(float dt) = 0;

protected:
    Unit* unit_;
    Order* previous_;

    friend class OrderList;
};

class MoveOrder : public Order
{
public:
    MoveOrder(const Vec2& target_position);

    ~MoveOrder() override = default;

    // Order
    void draw(sf::RenderWindow* window) override;
    bool tick(float dt) override;

private:
    Vec2 target_position_;

private:
    Vec2 startPosition() const;
    Vec2 remaining() const;
};

class OrderList {
public:
    OrderList(Unit* unit);
    OrderList(Unit* unit, List<UniquePtr<Order>> orders);

    void add(UniquePtr<Order> order, bool queue);

    void draw(sf::RenderWindow* window);
    void tick(float dt);

private:
    Unit* unit_;
    List<UniquePtr<Order>> orders_;
};