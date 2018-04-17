#include "Common.h"
#include "world/State.h"
#include "world/World.h"

State::State(sf::Color colour, const String& name, const HashSet<Map::Site*>& land) : colour_(colour), name_(name), land_(land), highlighted_{false} {
    colour_.a = 100;
    gui_name_.setString(name);

    // Calculate centre.
    centre_ = {0.0f, 0.0f};
    for (auto& tile : land_) {
        centre_ += tile->centre;
    }
    centre_ /= (float)land_.size();

    gui_name_.setPosition(toSFML(centre_));
    gui_name_.setCharacterSize(20);
}

void State::setName(const String &name) {
    name_ = name;
    gui_name_.setString(name);
}

void State::setHighlighted(bool highlighted) {
    highlighted_ = highlighted;
}

void State::addLandTile(Map::Site *tile) {
    if (tile->owning_state) {
        tile->owning_state->removeLandTile(tile);
    }
    land_.insert(tile);
    tile->owning_state = this;
}

void State::removeLandTile(Map::Site *tile) {
    land_.erase(tile);
    tile->owning_state = nullptr;
}

void State::draw(sf::RenderWindow* window, World* world) {
    sf::Color colour = colour_;
    if (!highlighted_) {
        colour.a = 40;
    }
    for (const auto tile : land_) {
        world->drawTile(window, *tile, colour);
        //world->drawTileEdge(window, *tile, sf::Color(colour_.r, colour_.g, colour_.b, 40));
    }
}

void State::drawBorders(sf::RenderWindow* window, World* world) {
    // Build edge list containing state boundaries.
    Vector<Vector<Map::GraphEdge*>> exclave_boundaries = unorderedBoundary();

    // Sort boundaries by joining vertices together.
    for (auto& boundaries : exclave_boundaries) {
        Vector<Vec2> points;
        for (auto& edge : boundaries) {
            points.emplace_back(edge->points.front());
            for (int p = 1; p < (edge->points.size() - 1); ++p) {
                points.emplace_back(edge->points[p]);
                points.emplace_back(edge->points[p]);
            }
            points.emplace_back(edge->points.back());
        }

        // Draw border.
        HSVColour border_colour = colour_;
        border_colour.s = 0.1f;
        border_colour.v = 1.0f;
        border_colour.a = 1.0f;

        world->drawLineList(window, points, border_colour);
    }
}

void State::drawOverlays(sf::RenderWindow *window, World *world) {
    window->draw(gui_name_);
}

Vector<Vector<Map::GraphEdge*>> State::unorderedBoundary() const {
    // Build edge list containing state boundaries.
    Vector<Vector<Map::GraphEdge*>> exclave_boundaries;
    exclave_boundaries.emplace_back();
    for (auto &state_tile : land_) {
        for (int i = 0; i < state_tile->edges.size(); ++i) {
            if (land_.count(state_tile->edges[i].neighbour) == 0) {
                exclave_boundaries.back().push_back(&state_tile->edges[i]);
            }
        }
    }
    return exclave_boundaries;
}

const HashSet<Map::Site*>& State::land() const {
    return land_;
}

const Vec2 State::midpoint() const {
    return centre_;
}
