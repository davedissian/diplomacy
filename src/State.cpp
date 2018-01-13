#include "Common.h"
#include "State.h"

State::State(sf::Color colour, const String& name, const HashSet<Tile*>& land) : colour_(colour), name_(name), land_(land) {
}

void State::addLandTile(Tile *tile) {
    land_.insert(tile);
}

void State::removeLandTile(Tile *tile) {
    land_.erase(tile);
}

void State::draw(sf::RenderWindow* window, World* world) {
    for (const auto tile : land_) {
        world->drawTile(window, *tile, colour_);
        //world->drawTileEdge(window, *tile, sf::Color(colour_.r, colour_.g, colour_.b, 40));
    }
}

void State::drawBorders(sf::RenderWindow* window, World* world) {
    // Build edge list containing state boundaries.
    Vector<Vector<VoronoiGraph::OrderedEdge*>> exclave_boundaries = unorderedBoundary();

    // Sort boundaries by joining vertices together.
    for (auto& boundaries : exclave_boundaries) {
        for (int i = 0; i < boundaries.size(); ++i) {
            // Consider all edges after this one.
            for (int j = i + 1; j < boundaries.size(); ++j) {
                // If edge[j] joins the end of edge[i], then move edge[j] to i + 1.
                if (vecEqual(boundaries[j]->v0(), boundaries[i]->v1(), VORONOI_EPSILON)) {
                    auto *temp = boundaries[i + 1];
                    boundaries[i + 1] = boundaries[j];
                    boundaries[j] = temp;
                    break;
                }
            }
        }

        // Convert into list of points.
        Vector<Vec2> ribbon_points;
        ribbon_points.reserve(boundaries.size());
        for (auto &edge : boundaries) {
            ribbon_points.push_back(edge->v0());
        }

        // Draw ribbon.
        HSVColour border_colour = colour_;
        border_colour.v = 0.4f;
        border_colour.a = 1.0f;
        world->drawJoinedRibbon(window, ribbon_points, 0.0f, 5.0f, border_colour);
    }
}

Vector<Vector<VoronoiGraph::OrderedEdge*>> State::unorderedBoundary() const {
    // Build edge list containing state boundaries.
    Vector<Vector<VoronoiGraph::OrderedEdge*>> exclave_boundaries;
    exclave_boundaries.emplace_back();
    for (auto &state_tile : land_) {
        for (int i = 0; i < state_tile->neighbours.size(); ++i) {
            if (land_.count(state_tile->neighbours[i]) == 0) {
                exclave_boundaries.back().push_back(&state_tile->edges[i]);
            }
        }
    }
    return exclave_boundaries;
}

const HashSet<Tile*>& State::land() const {
    return land_;
}
