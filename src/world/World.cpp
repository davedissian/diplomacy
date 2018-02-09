#include "Common.h"
#include "world/World.h"
#include "world/Map.h"
#include "world/State.h"

World::World(int num_points, const Vec2& min, const Vec2& max) {
    const int relax_count = 20;

    // Seed RNG.
    std::uniform_real_distribution<> dist(0, 1);
    RngType::result_type const seedval = 0xDEADBEEF; // TODO: get this from somewhere
    rng_.seed(seedval);

    // Generate points for voronoi diagram.
    Vector<Vec2> points;
    for (int i = 0; i < num_points; ++i) {
        points.emplace_back(dist(rng_) * (max.x - min.x) + min.x, dist(rng_) * (max.y - min.y) + min.y);
    }

    // Build voronoi diagram and relax points using Lloyds Algorithm.
    voronoi::Voronoi voronoi(points, min.x, max.x, min.y, max.y);
    for (int i = 0; i < relax_count; ++i) {
        Vector<Vec2> centres;
        for (auto& site : voronoi.sites) {
            Vec2 centre = {0.0f, 0.0f};
            for (auto& edge : site.edges) {
                // An edge begins at (e.x, e.y) and ends at (e.z, e.w).
                centre += Vec2(edge.x, edge.y);
                centre += Vec2(edge.z, edge.w);
            }
            centre /= float(site.edges.size() * 2);
            centres.emplace_back(centre);
        }
        points = centres;
        voronoi = voronoi::Voronoi(points, min.x, max.x, min.y, max.y);
    }

    // Create map.
    map_ = make_unique<Map>(voronoi, min, max, rng_);
    unclaimed_tiles_.reserve(map_->tiles().size());
    for (auto& tile : map_->tiles()) {
        if (tile.usable) {
            unclaimed_tiles_.insert(&tile);
        }
    }
}


void World::generateStates(int count, int max_size) {
    for (int i = 0; i < count; ++i) {
        std::uniform_int_distribution<> tile_id_dist(0, (int)map_->tiles().size() - 1);

        // Take a tile as the starting land.
        HashSet<Map::Tile*> starting_land = {&map_->tiles()[tile_id_dist(rng_)]};
        unclaimed_tiles_.erase(*starting_land.begin());

        // Form a state here.
        std::uniform_real_distribution<float> hue_dist(0.0f, 360.0f);
        HSVColour country_colour{hue_dist(rng_), 0.8f, 0.7f, 0.8f};
        states_[i] = make_unique<State>(country_colour, "Generated State " + std::to_string(i), starting_land);

        // Try and take up to 'start_size' tiles.
        for (int j = 0; j < max_size; ++j) {
            if (!growState(states_[i].get())) {
                break;
            }
        }
    }
}

void World::fillStates(int count) {
    const int start_size = 40;
    for (int i = 0; i < count; ++i) {
        std::uniform_int_distribution<> tile_id_dist(0, (int)unclaimed_tiles_.size() - 1);

        // Take a tile as the starting land.
        int starting_tile_index = tile_id_dist(rng_);
        HashSet<Map::Tile*> starting_land = {*std::next(unclaimed_tiles_.begin(), starting_tile_index)};
        unclaimed_tiles_.erase(*starting_land.begin());

        // Form a state here.
        std::uniform_real_distribution<float> hue_dist(0.0f, 360.0f);
        HSVColour country_colour{hue_dist(rng_), 0.6f, 0.8f, 0.5f};
        states_[i] = make_unique<State>(country_colour, "Generated State " + std::to_string(i), starting_land);
    }

    // Grow each state until none can grow any longer.
    bool should_grow_states = true;
    while (should_grow_states) {
        bool state_grew = false;
        for (auto &state : states_) {
            state_grew |= growState(state.second.get());
        }
        if (!state_grew) {
            should_grow_states = false;
        }
    }
}

void World::draw(sf::RenderWindow* window) {
    // Draw map.
    for (auto& tile : map_->tiles()) {
        drawTile(window, tile, sf::Color(40, 40, 40));
        drawTileEdge(window, tile, sf::Color(80, 80, 80, 80));
    }

    // Draw states.
    for (auto& state_pair : states_) {
        state_pair.second->draw(window, this);
    }
    for (auto& state_pair : states_) {
        state_pair.second->drawBorders(window, this);
    }
}

void World::drawTile(sf::RenderWindow* window, const Map::Tile& tile, sf::Color colour) {
    sf::VertexArray tile_geometry(sf::Triangles);
    for (int i = 0; i < tile.edges.size(); ++i) {
        for (int p = 0; p < tile.edges[i].points.size() - 1; p++) {
            tile_geometry.append(sf::Vertex(toSFML(tile.centre), colour));
            tile_geometry.append(sf::Vertex(toSFML(tile.edges[i].points[p]), colour));
            tile_geometry.append(sf::Vertex(toSFML(tile.edges[i].points[p + 1]), colour));
        }
    }
    window->draw(tile_geometry);
}

void World::drawTileEdge(sf::RenderWindow *window, const Map::Tile &tile, sf::Color colour) {
    // Convert into list of points.
    Vector<Vec2> ribbon_points;
    ribbon_points.reserve(tile.edges.size());
    for (auto &edge : tile.edges) {
        for (int i = 0; i < edge.points.size() - 1; i++) {
            ribbon_points.push_back(edge.points[i]);
        }
    }

    // Draw ribbon.
    drawJoinedRibbon(window, ribbon_points, 0.0f, 1.5f, colour);
}

void World::drawJoinedRibbon(sf::RenderWindow *window, const Vector<Vec2>& points, float inner_thickness,
                             float outer_thickness, const sf::Color& colour) {
    // Draw border using a ribbon.
    if (points.size() > 2) {
        sf::VertexArray border(sf::Quads, points.size() * 4);

        // Generate ribbon edges from the cycle of points.
        Vector<Pair<Vec2, Vec2>> ribbon_edges;
        int num_points = (int)points.size();
        for (int i = 0; i < num_points; ++i) {
            // To generate pair of ribbon points about point i, we need to consider points: i-1 -> i -> i+1.
            const Vec2& a = points[(i - 1 + num_points) % num_points];
            const Vec2& b = points[i];
            const Vec2& c = points[(i + 1) % num_points];
            Vec2 t_ab = glm::normalize(Vec2{a.y - b.y, b.x - a.x});
            Vec2 t_bc = glm::normalize(Vec2{b.y - c.y, c.x - b.x});
            const Vec2 ribbon_first = intersection(
                    a + t_ab * outer_thickness, b + t_ab * outer_thickness,
                    b + t_bc * outer_thickness, c + t_bc * outer_thickness);
            const Vec2 ribbon_second = intersection(
                    a - t_ab * inner_thickness, b - t_ab * inner_thickness,
                    b - t_bc * inner_thickness, c - t_bc * inner_thickness);
            ribbon_edges.emplace_back(ribbon_first, ribbon_second);
        }

        // Join ribbon edges with quads.
        for (int i = 0; i < ribbon_edges.size(); ++i) {
            border[i * 4].position = toSFML(ribbon_edges[i].first);
            border[i * 4].color = colour;
            border[i * 4 + 1].position = toSFML(ribbon_edges[(i + 1) % ribbon_edges.size()].first);
            border[i * 4 + 1].color = colour;
            border[i * 4 + 2].position = toSFML(ribbon_edges[(i + 1) % ribbon_edges.size()].second);
            border[i * 4 + 2].color = colour;
            border[i * 4 + 3].position = toSFML(ribbon_edges[i].second);
            border[i * 4 + 3].color = colour;
        }
        window->draw(border);
    }
}

const Vector<Map::Tile>& World::mapTiles() const {
    return map_->tiles();
}

const HashMap<int, SharedPtr<State>> &World::states() const {
    return states_;
}

WeakPtr<State> World::getStateById(int id) const {
    auto it = states_.find(id);
    if (it != states_.end()) {
        return it->second;
    }
    return {};
}

bool World::growState(State *state) {
    auto border = state->unorderedBoundary(); // This will always contain a single exclave, the mainland.

    // Map border points to unclaimed land.
    Vector<Map::Tile*> unclaimed_border_tiles;
    for (auto& edge : border[0]) {
        if (unclaimed_tiles_.count(edge->shared_edge->d0) == 1) {
            unclaimed_border_tiles.push_back(edge->shared_edge->d0);
        }
        if (unclaimed_tiles_.count(edge->shared_edge->d1) == 1) {
            unclaimed_border_tiles.push_back(edge->shared_edge->d1);
        }
    }

    // If none are left, give up.
    if (unclaimed_border_tiles.empty()) {
        return false;
    }

    // Claim a random border tile.
    std::uniform_int_distribution<> random_edge_dist(0, (int)unclaimed_border_tiles.size() - 1);
    Map::Tile* next_tile = unclaimed_border_tiles[random_edge_dist(rng_)];
    state->addLandTile(next_tile);
    unclaimed_tiles_.erase(next_tile);
    return true;
}
