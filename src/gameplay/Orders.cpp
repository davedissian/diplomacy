#include "Common.h"
#include "Orders.h"
#include "Unit.h"
#include "RenderContext.h"

Order::Order() : unit_{nullptr}, previous_{nullptr} {
}

MoveOrder::MoveOrder(const Vec2& target_position): target_position_{target_position} {
}

void MoveOrder::draw(RenderContext& ctx) {
    Vec2 rem = remaining();
    sf::RectangleShape travel_line;
    travel_line.setSize({glm::length(rem), 3.0f});
    travel_line.setPosition(toSFML(startPosition()));
    travel_line.setRotation(atan2(rem.y, rem.x) * RAD_TO_DEG);
    travel_line.setFillColor(sf::Color(255, 255, 255, 120));
    ctx.window->draw(travel_line);
}

bool MoveOrder::tick(float dt) {
    Vec2 rem = remaining();
    float rem_length = glm::length(rem);
    if (rem_length < 5.0f) {
        return false;
    }

    unit_->stepTowards(dt, rem / rem_length, 1.0f);
    return true;
}

Vec2 MoveOrder::startPosition() const {
    if (previous_) {
        MoveOrder* previous_move = dynamic_cast<MoveOrder*>(previous_);
        if (previous_move) {
            return previous_move->target_position_;
        }
    }
    return unit_->position();
}

Vec2 MoveOrder::remaining() const {
    return target_position_ - startPosition();
}

OrderList::OrderList(Unit* unit) : OrderList(unit, List<UniquePtr<Order>>{}) {
}

OrderList::OrderList(Unit* unit, List<UniquePtr<Order>> orders) : unit_{unit}, orders_(std::move(orders)) {
}

void OrderList::add(UniquePtr<Order> order, bool queue) {
    order->unit_ = unit_;
    if (queue) {
        if (!orders_.empty()) {
            order->previous_ = orders_.back().get();
        }
    } else {
        orders_.clear();
    }
    orders_.emplace_back(std::move(order));
}

void OrderList::draw(RenderContext& ctx) {
    for (auto it = orders_.rbegin(); it != orders_.rend(); ++it) {
        (*it)->draw(ctx);
    }
}

void OrderList::tick(float dt) {
    // Process first order.
    if (!orders_.empty()) {
        // Once that order has finished, pop it.
        if (!orders_.front()->tick(dt)) {
            orders_.pop_front();
            if (!orders_.empty()) {
                orders_.front()->previous_ = nullptr;
            }
        }
    }
}
