#include "Common.h"
#include "Orders.h

OrderList::OrderList(Unit* unit) : OrderList(unit, Queue<UniquePtr<Order>>{}) {
}

OrderList::OrderList(Unit* unit, Queue<UniquePtr<Order>> orders) : unit_{unit}, current_order_(nullptr), orders_(orders) {
}

void OrderList::enqueue(UniquePtr<Order> order) {
    order->unit_ = unit_;
    orders_.emplace(std::move(order));
}

void OrderList::tick(float dt) {
    // Fetch a new order.
    if (!current_order_ && orders_.size() > 0) {
        current_order_ = std::move(orders_.front());
        orders_.pop();
    }

    // If we're processing an order.
    if (current_order_) {
        // Process it. If it's finished processing, clear it.
        if (!current_order_->tick(dt)) {
            current_order_ = nullptr;
        }
    }
}
