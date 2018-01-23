#include "Common.h"
#include "World.h"

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

    Vec2 mapScreenToWorld(const Vec2i& screen_position) const;

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
    const int world_size = 0;
    switch (world_size)
    {
    case 0: world_ = make_unique<World>(400, Vec2{0.0f, 0.0f}, Vec2{1800.0f, 1800.0f}); break;
    case 1: world_ = make_unique<World>(800, Vec2{0.0f, 0.0f}, Vec2{2400.0f, 2400.0f}); break;
    case 2: world_ = make_unique<World>(1600, Vec2{0.0f, 0.0f}, Vec2{4800.0f, 2400.0f}); break;
    }

    // Set up viewport.
    target_centre_ = {0.0f, 0.0f};
    viewport_.setCenter({0.0f, 0.0f});
    target_size_ = window_size;
    viewport_.setSize(toSFML(window_size));

    // Deal with retina display.
#ifdef __APPLE__
    ImGui::GetIO().DisplayFramebufferScale = ImVec2(2.0f, 2.0f);
    window_size *= 2;
#endif

    screen_size_ = window_size;
    window_ = make_unique<sf::RenderWindow>(sf::VideoMode((u32)window_size.x, (u32)window_size.y), "Diplomacy RTS");
    window_->setVerticalSyncEnabled(true);
    ImGui::SFML::Init(*window_);

    sf::Clock delta_clock;
    sf::Time dt_time;
    float dt = 1.0f / 60.0f;
    while (window_->isOpen()) {
        sf::Event event;
        while (window_->pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
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
        ImGui::SFML::Update(*window_, dt_time);

        // Update camera.
        target_centre_ += camera_movement_speed_ * dt;
        Vec2 current_centre = fromSFML(viewport_.getCenter());
        viewport_.setCenter(toSFML(damp(current_centre, target_centre_, 0.4f, 0.1f, dt)));
        Vec2 current_size = fromSFML(viewport_.getSize());
        viewport_.setSize(toSFML(damp(current_size, target_size_, 0.4f, 0.1f, dt)));

        // Draw.
        window_->setView(viewport_);
        window_->clear(sf::Color::Black);
        world_->draw(window_.get());
        ImGui::SFML::Render(*window_);
        window_->display();

        // Update frame timer.
        dt_time = delta_clock.restart();
        dt = dt_time.asSeconds();
    }

    return 0;
}

void Game::handleMouseMoved(float dt, sf::Event::MouseMoveEvent &e) {
    Vec2i current_mouse_position = {e.x, e.y};
    Vec2i mouse_movement = current_mouse_position - last_mouse_position_;
    last_mouse_position_ = current_mouse_position;

    // Highlight tile underneath cursor.
    Vec2 proj_mouse_position = mapScreenToWorld(current_mouse_position);
    world_->onMouseMove(proj_mouse_position);

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
    const float scale = 1.5f;
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
        // Lerp between current_centre and mouse_centre.
        // A factor of 0 means we arrive at mouse_centre. 1 means no change from current position.
        new_centre = lerp(mapScreenToWorld({e.x, e.y}), current_centre, factor);
    }

    target_size_ = new_size;
    target_centre_ = new_centre;
}

Vec2 Game::mapScreenToWorld(const Vec2i &screen_position) const {
    return fromSFML(window_->mapPixelToCoords({screen_position.x, screen_position.y}, viewport_));
};

int main() {
    Game game;
    return game.run({1280, 800});
};
