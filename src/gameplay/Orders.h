#pragma once

class Unit;

class Order {
public:
    Order() = default;
    virtual ~Order() = default;

    // True: completed. False: Requires more processing.
    virtual bool tick(float dt) = 0;

protected:
    Unit* unit_;

    friend class OrderList;
};

class OrderList {
public:
    OrderList(Unit* unit);
    OrderList(Unit* unit, Queue<UniquePtr<Order>> orders);

    void enqueue(UniquePtr<Order> order);
    void tick(float dt);

private:
    Unit* unit_;
    UniquePtr<Order> current_order_;
    Queue<UniquePtr<Order>> orders_;
};