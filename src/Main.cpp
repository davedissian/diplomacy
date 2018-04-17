#include "Common.h"
#include "world/World.h"
#include "player/Player.h"
#include "gameplay/Squad.h"
#include "player/LocalController.h"

#define DEBUG_GUI

const int BOUNDARY_SIZE = 100;

class Game;

class GameState
{
public:
    GameState(Game* game);
    virtual ~GameState() = default;

    virtual void tick(float dt) {}
    virtual void draw(sf::RenderWindow* window) {}

    virtual void handleKey(float dt, sf::Event::KeyEvent& e, bool pressed) {}
    virtual void handleMouseMoved(float dt, sf::Event::MouseMoveEvent& e) {}
    virtual void handleMouseButton(float dt, sf::Event::MouseButtonEvent& e, MouseButtonState state) {}
    virtual void handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent& e) {}

    virtual sf::View& viewport() { return viewport_; }
    virtual const sf::View& viewport() const { return viewport_; }

protected:
    Game* game_;
    sf::View viewport_;
};

class MainGameState : public GameState
{
public:
    MainGameState(Game* game);
    ~MainGameState() override;

    void tick(float dt) override;
    void draw(sf::RenderWindow* window) override;

    void handleKey(float dt, sf::Event::KeyEvent& e, bool pressed) override;
    void handleMouseMoved(float dt, sf::Event::MouseMoveEvent& e) override;
    void handleMouseButton(float dt, sf::Event::MouseButtonEvent& e, MouseButtonState state) override;
    void handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent& e) override;

private:
    Vec2i last_mouse_position_;
    Vec2 camera_movement_speed_;

    Vec2 target_centre_;
    Vec2 target_size_;

    UniquePtr<World> world_;
    const Map::Site* selected_tile_;

    // Players.
    Vector<SharedPtr<Player>> players_;

    // Local controller.
    UniquePtr<LocalController> local_controller_;

    // Units.
    Vector<SharedPtr<Unit>> units_;
    bool show_orders_;
};

class MenuGameState : public GameState
{
public:
    MenuGameState(Game* game);
    ~MenuGameState() override;

    void tick(float dt) override;
};

class Game {
public:
    Game();

    int run(Vec2i window_size);

    Vec2 mapScreenToWorld(const Vec2i& screen_position) const;

    const Vec2i& screenSize() const {
        return screen_size_;
    }

    template <typename S>
    void switchTo() {
        current_state_ = make_unique<S>(this);
    }

    void exit() {
        window_->close();
    }

private:
    Vec2i screen_size_;
    UniquePtr<sf::RenderWindow> window_;

    UniquePtr<GameState> current_state_;
};

GameState::GameState(Game* game): game_{game}
{
    viewport_.setSize(toSFML(game_->screenSize()));
}

MainGameState::MainGameState(Game* game) : GameState(game), camera_movement_speed_{0.0f, 0.0f}, selected_tile_{nullptr}, show_orders_{false} {
    const int world_size_preset = 0;
    const int num_players = 8;

    switch (world_size_preset)
    {
    case 0: world_ = make_unique<World>(400, Vec2{ 0.0f, 0.0f }, Vec2{ 1800.0f, 1800.0f }); break;
    case 1: world_ = make_unique<World>(800, Vec2{ 0.0f, 0.0f }, Vec2{ 2400.0f, 2400.0f }); break;
    case 2: world_ = make_unique<World>(1600, Vec2{ 0.0f, 0.0f }, Vec2{ 4800.0f, 2400.0f }); break;
    }

    // Create states and set up players to take ownership of states.
    world_->fillStates(num_players);
    auto states = world_->states();
    for (auto state_pair : states) {
        // Set up units.
        Vector<SharedPtr<Unit>> units;
        units.emplace_back(std::make_shared<Squad>(state_pair.second->midpoint()));
        units_.insert(units_.end(), units.begin(), units.end());

        // Create player.
        WeakPtr<State> state = state_pair.second;
        Vector<WeakPtr<Unit>> units_weak;
        for (auto& ptr : units) {
            units_weak.push_back(WeakPtr<Unit>(ptr));
        }
        players_.emplace_back(std::make_shared<Player>(String("Player ") + std::to_string(state_pair.first), state, units_weak));

        // Name state.
        //state_pair.second->setName(String("State ") + std::to_string(state_pair.first));
    }

    // Set up local controller.
    local_controller_ = make_unique<LocalController>();
    local_controller_->possess(players_[0].get());

    // Set up viewport.
    target_centre_ = {0.0f, 0.0f};
    viewport_.setCenter({0.0f, 0.0f});
    target_size_ = game_->screenSize();
}

MainGameState::~MainGameState() {
}

void MainGameState::tick(float dt) {
    // Update camera.
    target_centre_ += camera_movement_speed_ * dt;
    Vec2 current_centre = fromSFML(viewport_.getCenter());
    viewport_.setCenter(toSFML(damp(current_centre, target_centre_, 0.4f, 0.1f, dt)));
    Vec2 current_size = fromSFML(viewport_.getSize());
    viewport_.setSize(toSFML(damp(current_size, target_size_, 0.4f, 0.1f, dt)));

    // Update units.
    for (auto& unit : units_) {
        unit->tick(dt);
    }
}

void MainGameState::draw(sf::RenderWindow* window) {
    // Draw world.
    world_->draw(window);

    // Highlight selected tile.
    if (selected_tile_) {
        sf::Color selected_color(20, 50, 120, 120);
        sf::Color selected_edge_color(20, 50, 120, 200);
        world_->drawTile(window, *selected_tile_, selected_color);
        world_->drawTileEdge(window, *selected_tile_, selected_edge_color);

#ifdef DEBUG_GUI
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(700, 250));
        ImGui::Begin("Selected Tile");
        ImGui::Text("Centre: %f %f", selected_tile_->centre.x, selected_tile_->centre.y);
        ImGui::Text("Unusable? %s", selected_tile_->usable ? "true" : "false");
        for (int i = 0; i < selected_tile_->edges.size(); ++i) {
            auto& e = selected_tile_->edges[i];
            auto& n = e.neighbour;
            float angle1 = (float)selected_tile_->vertexAngle(e.points.front());
            float angle2 = (float)selected_tile_->vertexAngle(e.points.back());
            if (n != nullptr) {
                ImGui::Text("- Neighbour %d (edge %.0f %.0f (%.1f) to %.0f %.0f (%.1f) diff: %.1f) - centre %.0f %.0f",
                    i,
                    e.points.front().x, e.points.front().y, angle1,
                    e.points.back().x, e.points.back().y, angle2, angle2 - angle1,
                    n->centre.x, n->centre.y);
            }
            else {
                ImGui::Text("- Neighbour %d (edge %.0f %.0f (%.1f) to %.0f %.0f (%.1f) diff: %.1f) - none",
                    i,
                    e.points.front().x, e.points.front().y, angle1,
                    e.points.back().x, e.points.back().y, angle2, angle2 - angle1);
            }
        }
        ImGui::End();
#endif
    }

    // Draw units.
    for (auto& unit : units_) {
        unit->draw(window, world_.get());
    }

    // Draw overlays.
    if (show_orders_) {
        for (auto& unit : units_) {
            unit->drawOrderOverlay(window);
        }
    }
}

void MainGameState::handleKey(float dt, sf::Event::KeyEvent& e, bool pressed) {
    if (e.code == sf::Keyboard::LShift) {
        show_orders_ = pressed;
    }
}

void MainGameState::handleMouseMoved(float dt, sf::Event::MouseMoveEvent &e) {
    Vec2i current_mouse_position = { e.x, e.y };
    Vec2i mouse_movement = current_mouse_position - last_mouse_position_;
    last_mouse_position_ = current_mouse_position;

    // Highlight tile underneath cursor.
    Vec2 proj_mouse_position = game_->mapScreenToWorld(current_mouse_position);
    float nearest_distance_sq = std::numeric_limits<float>::infinity();
    for (auto& tile : world_->mapTiles()) {
        float distance = glm::distance2(proj_mouse_position, tile.centre);
        if (distance < nearest_distance_sq) {
            nearest_distance_sq = distance;
            selected_tile_ = &tile;
        }
    }

    // If the mouse cursor reaches the boundary of the screen, move in that direction, scaled by distance.
    camera_movement_speed_ = { 0.0f, 0.0f };
    const Vec2 speed = fromSFML(viewport_.getSize()) * 0.5f;
    // Right side.
    if (current_mouse_position.x > game_->screenSize().x - BOUNDARY_SIZE) {
        int distance = game_->screenSize().x - current_mouse_position.x;
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
    if (current_mouse_position.y > game_->screenSize().y - BOUNDARY_SIZE) {
        int distance = game_->screenSize().y - current_mouse_position.y;
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

void MainGameState::handleMouseButton(float dt, sf::Event::MouseButtonEvent &e, MouseButtonState state) {
    if (state == MouseButtonState::Pressed && e.button == sf::Mouse::Right) {
        units_[0]->addOrder(make_unique<MoveOrder>(game_->mapScreenToWorld(last_mouse_position_)), show_orders_);
    }
}

void MainGameState::handleMouseScroll(float dt, sf::Event::MouseWheelScrollEvent &e) {
    const float dead_zone = 0.1f;
    const float scale = 1.5f;
    float factor;
    if (e.delta > dead_zone) {
        factor = 1.0f / scale;
    }
    else if (e.delta < -dead_zone) {
        factor = scale;
    }
    else {
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
        new_centre = lerp(game_->mapScreenToWorld({ e.x, e.y }), current_centre, factor);
    }

    target_size_ = new_size;
    target_centre_ = new_centre;
}

MenuGameState::MenuGameState(Game* game) : GameState(game) {
}

MenuGameState::~MenuGameState() {
}

void MenuGameState::tick(float dt) {
    auto window_size = ImVec2(900, 300);
    ImGui::SetNextWindowPos({(game_->screenSize().x - window_size.x) / 2, (game_->screenSize().y - window_size.y) / 2});
    ImGui::SetNextWindowSize(window_size);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
    ImGui::Begin("", 0, flags);
    ImGui::Text("DIPLOMACY");
    if (ImGui::Button("Begin", {150.0f, 25.0f})) {
        game_->switchTo<MainGameState>();
    }
    ImGui::Button("Flag Editor", {150.0f, 25.0f});
    if (ImGui::Button("Exit", {150.0f, 25.0f})) {
        game_->exit();
    }
    ImGui::End();
}

Game::Game() {}

int Game::run(Vec2i window_size) {

    // Deal with retina display.
#ifdef __APPLE__
    ImGui::GetIO().DisplayFramebufferScale = ImVec2(2.0f, 2.0f);
    window_size *= 2;
#endif

    screen_size_ = window_size;
    window_ = make_unique<sf::RenderWindow>(sf::VideoMode((u32)window_size.x, (u32)window_size.y), "Diplomacy RTS");
    window_->setVerticalSyncEnabled(true);
    ImGui::SFML::Init(*window_);

    current_state_ = make_unique<MenuGameState>(this);

    sf::Clock delta_clock;
    sf::Time dt_time;
    float dt = 1.0f / 60.0f;
    while (window_->isOpen()) {
        sf::Event event;
        while (window_->pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::KeyPressed) {
                current_state_->handleKey(dt, event.key, true);
            }
            if (event.type == sf::Event::KeyReleased) {
                current_state_->handleKey(dt, event.key, false);
            }
            if (event.type == sf::Event::MouseMoved) {
                current_state_->handleMouseMoved(dt, event.mouseMove);
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                current_state_->handleMouseButton(dt, event.mouseButton, MouseButtonState::Pressed);
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                current_state_->handleMouseButton(dt, event.mouseButton, MouseButtonState::Released);
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                current_state_->handleMouseScroll(dt, event.mouseWheelScroll);
            }
            if (event.type == sf::Event::Closed) {
                window_->close();
            }
        }
        ImGui::SFML::Update(*window_, dt_time);

        // Update.
        current_state_->tick(dt);

        // Draw.
        window_->setView(current_state_->viewport());
        window_->clear(sf::Color::Black);
        current_state_->draw(window_.get());
        ImGui::SFML::Render(*window_);
        window_->display();

        // Update frame timer.
        dt_time = delta_clock.restart();
        dt = dt_time.asSeconds();
    }

    return 0;
}

Vec2 Game::mapScreenToWorld(const Vec2i &screen_position) const {
    return fromSFML(window_->mapPixelToCoords({screen_position.x, screen_position.y}, current_state_->viewport()));
}

int main() {
    Game game;
    return game.run({1280, 800});
};
