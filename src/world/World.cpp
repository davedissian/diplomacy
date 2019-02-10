#include "Common.h"
#include "world/World.h"
#include "world/Map.h"
#include "world/State.h"

World::World(int num_points, const Vec2& min, const Vec2& max) {
    // Create map.
    map_ = make_unique<Map>(num_points, min, max, rng_);
    unclaimed_tiles_.reserve(map_->sites().size());
    for (auto& tile : map_->sites()) {
        if (tile.usable) {
            unclaimed_tiles_.insert(&tile);
        }
    }
}


void World::generateStates(int count, int max_size) {
    for (int i = 0; i < count; ++i) {
        std::uniform_int_distribution<> tile_id_dist(0, (int) map_->sites().size() - 1);

        // Take a tile as the starting land.
        HashSet<Map::Site*> starting_land = {&map_->sites()[tile_id_dist(rng_)]};
        unclaimed_tiles_.erase(*starting_land.begin());

        // Form a state here.
        std::uniform_real_distribution<float> hue_dist(0.0f, 360.0f);
        HSVColour country_colour{hue_dist(rng_), 0.8f, 0.7f, 0.8f};
        states_[i] = make_unique<State>(country_colour, "Generated State " + std::to_string(i), starting_land);

        // Try and take up to 'start_size' sites.
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
        HashSet<Map::Site*> starting_land = {*std::next(unclaimed_tiles_.begin(), starting_tile_index)};
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

void World::draw(RenderContext& ctx) {
    // Draw map.
    for (auto& tile : map_->sites()) {
        drawTile(ctx, tile, sf::Color(40, 40, 40));
        drawTileEdge(ctx, tile, sf::Color(80, 80, 80, 80));
    }

    // Draw states.
    for (auto& state_pair : states_) {
        state_pair.second->draw(ctx, false);
    }
    /*
    for (auto& state_pair : states_) {
        state_pair.second->drawBorders(ctx);
    }
     */
    for (auto& state_pair : states_) {
        state_pair.second->drawOverlays(ctx);
    }
}

void World::drawTile(RenderContext& ctx, const Map::Site& tile, sf::Color colour) {
    sf::VertexArray tile_geometry(sf::Triangles);
    for (int i = 0; i < tile.edges.size(); ++i) {
        for (int p = 0; p < tile.edges[i].points.size() - 1; p++) {
            tile_geometry.append(sf::Vertex(toSFML(tile.centre), colour));
            tile_geometry.append(sf::Vertex(toSFML(tile.edges[i].points[p]), colour));
            tile_geometry.append(sf::Vertex(toSFML(tile.edges[i].points[p + 1]), colour));
        }
    }
    ctx.window->draw(tile_geometry);
}

void World::drawTileEdge(RenderContext& ctx, const Map::Site &tile, sf::Color colour) {
    // Convert into list of points.
    Vector<Vec2> ribbon_points;
    ribbon_points.reserve(tile.edges.size());
    for (auto &edge : tile.edges) {
        for (int i = 0; i < edge.points.size() - 1; i++) {
            ribbon_points.push_back(edge.points[i]);
        }
    }

    // Draw ribbon.
    drawJoinedRibbon(ctx, ribbon_points, 0.0f, 1.5f, colour);
}

void World::drawLineList(RenderContext& ctx, const Vector<Vec2>& points, const sf::Color & colour)
{
	if (points.size() < 2) {
		return;
	}
	Vector<sf::Vertex> line_list;
	line_list.reserve(points.size());
	for (int i = 0; i < points.size(); ++i) {
		line_list.emplace_back(toSFML(points[i]), colour);
	}
	ctx.window->draw(line_list.data(), line_list.size(), sf::Lines);
}

void World::drawJoinedRibbon(RenderContext& ctx, const Vector<Vec2>& points, float inner_thickness,
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
        ctx.window->draw(border);
    }
}

void World::drawBorder(RenderContext& ctx, Vector<Vector<Map::GraphEdge*>> list_of_boundaries, sf::Color colour)
{
	// Sort boundaries by joining vertices together.
	for (auto& boundaries : list_of_boundaries)
	{
		Vector<Vec2> points;
		for (auto& edge : boundaries)
		{
			points.emplace_back(edge->points.front());
			for (int p = 1; p < (edge->points.size() - 1); ++p)
			{
				points.emplace_back(edge->points[p]);
				points.emplace_back(edge->points[p]);
			}
			points.emplace_back(edge->points.back());
		}

		// Draw border.
		HSVColour border_colour = colour;
		border_colour.s = 0.1f;
		border_colour.v = 1.0f;
		border_colour.a = 1.0f;

		drawLineList(ctx, points, border_colour);
	}
}

Vector<Map::Site>& World::mapSites() {
    return map_->sites();
}

const Vector<Map::Site>& World::mapSites() const {
    return map_->sites();
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
    Vector<Map::Site*> unclaimed_border_tiles;
    for (auto& e : border[0]) {
        if (unclaimed_tiles_.count(e->edge->d[0]) == 1) {
            unclaimed_border_tiles.push_back(e->edge->d[0]);
        }
        if (unclaimed_tiles_.count(e->edge->d[1]) == 1) {
            unclaimed_border_tiles.push_back(e->edge->d[1]);
        }
    }

    // If none are left, give up.
    if (unclaimed_border_tiles.empty()) {
        return false;
    }

    // Claim a random border tile.
    std::uniform_int_distribution<> random_edge_dist(0, (int)unclaimed_border_tiles.size() - 1);
    Map::Site* next_tile = unclaimed_border_tiles[random_edge_dist(rng_)];
    state->addLandTile(next_tile);
    unclaimed_tiles_.erase(next_tile);
    return true;
}
