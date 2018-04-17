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
    land_.insert(tile);
}

void State::removeLandTile(Map::Site *tile) {
    land_.erase(tile);
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
		/*
        for (int i = 0; i < boundaries.size(); ++i) {
            // Consider all edges after this one.
            for (int j = i + 1; j < boundaries.size(); ++j) {
                // If edge[j] joins the end of edge[i], then move edge[j] to i + 1.
                if (vecEqual(boundaries[j]->points.front(), boundaries[i]->points.back(), VORONOI_EPSILON)) {
                    auto *temp = boundaries[i + 1];
                    boundaries[i + 1] = boundaries[j];
                    boundaries[j] = temp;
                    break;
                }
            }
        }
		*/

        // Convert into list of points.
        Vector<Vec2> ribbon_points;
        ribbon_points.reserve(boundaries.size());
        for (auto &edge : boundaries) {
            for (int i = 0; i < edge->points.size(); i++) {
                ribbon_points.push_back(edge->points[i]);
            }
        }

        // Draw border.
        HSVColour border_colour = colour_;
        border_colour.v = 0.4f;
        border_colour.a = 1.0f;
        world->drawLineList(window, ribbon_points, border_colour);
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
