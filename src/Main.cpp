#include "Common.h"
#include "Map.h"

enum class MouseButtonState {
    Pressed,
    Released
};

const int BOUNDARY_SIZE = 100;

class Game {
public:
    Game();

    int run(Vec2i window_size);

    void handleMouseMoved(float dt, sf::Event::MouseMoveEvent& e);
    void handleMouseButton(float dt, sf::Event::MouseButtonEvent& e, MouseButtonState state);
    void handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent& e);

private:
    Vec2i screen_size_;
    Vec2i last_mouse_position_;
    Vec2 camera_movement_speed_;

    Vec2 target_centre_;
    Vec2 target_size_;

    sf::View viewport_;
    UniquePtr<sf::RenderWindow> window_;

    UniquePtr<World> world_;
};

Game::Game() : camera_movement_speed_(0.0f, 0.0f) {}

int Game::run(Vec2i window_size) {
    world_ = make_unique<World>();

    screen_size_ = window_size;
    window_ = make_unique<sf::RenderWindow>(sf::VideoMode((u32)window_size.x, (u32)window_size.y), "RTS Game");

    target_centre_ = {0.0f, 0.0f};
    viewport_.setCenter({0.0f, 0.0f});
    target_size_ = screen_size_;
    viewport_.setSize(toSFML(screen_size_));

    sf::Clock delta_clock;
    float dt = 1.0f / 60.0f;
    while (window_->isOpen()) {
        sf::Event event;
        while (window_->pollEvent(event)) {
            // Events.
            if (event.type == sf::Event::MouseMoved) {
                handleMouseMoved(dt, event.mouseMove);
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                handleMouseButton(dt, event.mouseButton, MouseButtonState::Pressed);
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                handleMouseButton(dt, event.mouseButton, MouseButtonState::Released);
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                handleMouseScroll(dt, event.mouseWheelScroll);
            }
            if (event.type == sf::Event::Closed) {
                window_->close();
            }
        }

        // Update camera.
        target_centre_ += camera_movement_speed_ * dt;
        Vec2 current_centre = fromSFML(viewport_.getCenter());
        viewport_.setCenter(toSFML(damp(current_centre, target_centre_, 0.5f, 0.1f, dt)));
        Vec2 current_size = fromSFML(viewport_.getSize());
        viewport_.setSize(toSFML(damp(current_size, target_size_, 0.5f, 0.1f, dt)));

        // Draw.
        window_->setView(viewport_);
        window_->clear(sf::Color::Black);
        world_->draw(window_.get());
        window_->display();

        // Update frame timer.
        dt = delta_clock.getElapsedTime().asSeconds();
        delta_clock.restart();
    }

    return 0;
}

void Game::handleMouseMoved(float dt, sf::Event::MouseMoveEvent &e) {
    Vec2i current_mouse_position = {e.x, e.y};
    Vec2i mouse_movement = current_mouse_position - last_mouse_position_;
    last_mouse_position_ = current_mouse_position;

    // If the mouse cursor reaches the boundary of the screen, move in that direction, scaled by distance.
    camera_movement_speed_ = {0.0f, 0.0f};
    const Vec2 speed = fromSFML(viewport_.getSize());
    // Right side.
    if (current_mouse_position.x > screen_size_.x - BOUNDARY_SIZE) {
        int distance = screen_size_.x - current_mouse_position.x;
        if (distance < 0) {
            distance = 0;
        }
        camera_movement_speed_.x += speed.x;
    }
    // Left side.
    if (current_mouse_position.x < BOUNDARY_SIZE) {
        int distance = current_mouse_position.x;
        if (distance < 0) {
            distance = 0;
        }
        camera_movement_speed_.x -= speed.x;
    }
    // Bottom.
    if (current_mouse_position.y > screen_size_.y - BOUNDARY_SIZE) {
        int distance = screen_size_.y - current_mouse_position.y;
        if (distance < 0) {
            distance = 0;
        }
        camera_movement_speed_.y += speed.y;
    }
    // Top.
    if (current_mouse_position.y < BOUNDARY_SIZE) {
        int distance = current_mouse_position.y;
        if (distance < 0) {
            distance = 0;
        }
        camera_movement_speed_.y -= speed.y;
    }
}

void Game::handleMouseButton(float dt, sf::Event::MouseButtonEvent &e, MouseButtonState state) {

}

void Game::handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent &e) {
    const float dead_zone = 0.1f;
    const float scale = 2.0f;
    float factor;
    if (e.delta > dead_zone) {
        factor = 1.0f / scale;
    } else if (e.delta < -dead_zone) {
        factor = scale;
    } else {
        return;
    }

    // Scale viewport size.
    Vec2 current_size = fromSFML(viewport_.getSize());
    Vec2 new_size = current_size * factor;

    // Calculate new position. If zooming in, move towards cursor, otherwise don't change.
    Vec2 current_centre = fromSFML(viewport_.getCenter());
    Vec2 new_centre = current_centre;
    if (factor < 1.0f) {
        Vec2 mouse_centre = fromSFML(window_->mapPixelToCoords({e.x, e.y}, viewport_));
        // Lerp between current_centre and mouse_centre.
        // A factor of 0 means we arrive at mouse_centre. 1 means no change from current position.
        new_centre = lerp(mouse_centre, current_centre, factor);
    }

    target_size_ = new_size;
    target_centre_ = new_centre;
};

int main() {
    Game game;
    return game.run({1600, 900});
};
