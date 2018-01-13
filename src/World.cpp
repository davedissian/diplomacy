#include "Common.h"
#include "World.h"
#include "State.h"

Vec2 &VoronoiGraph::OrderedEdge::v0() {
    return flipped ? edge->v1 : edge->v0;
}

const Vec2 &VoronoiGraph::OrderedEdge::v0() const {
    return flipped ? edge->v1 : edge->v0;
}

Vec2 &VoronoiGraph::OrderedEdge::v1() {
    return flipped ? edge->v0 : edge->v1;
}

const Vec2 &VoronoiGraph::OrderedEdge::v1() const {
    return flipped ? edge->v0 : edge->v1;
}

double VoronoiGraph::Tile::edgeAngle(const VoronoiGraph::Edge &e) {
    Vec2 centre_offset = (e.v0 + e.v1) * 0.5f - centre;
    return atan2(centre_offset.y, centre_offset.x);
}

double VoronoiGraph::Tile::vertexAngle(const Vec2 &v) {
    return atan2(v.y - centre.y, v.x - centre.x);
}

Pair<Vec2, Vec2> VoronoiGraph::Tile::orderedEdgeVertices(const OrderedEdge &e) {
    return {e.v0(), e.v1()};
}

VoronoiGraph::VoronoiGraph(const voronoi::Voronoi& voronoi) {
    // Build a list of edges.
    Vector<Edge> initial_edges;
    initial_edges.reserve(voronoi.sites.size() * 5); // A reasonable guess to avoid reallocations.
    tiles_.resize(voronoi.sites.size());
    int tile_counter = 0;
    for (auto& site : voronoi.sites) {
        Tile* tile = &tiles_[tile_counter++];
        tile->centre = site.coord;
        for (auto& edge : site.edges) {
            initial_edges.push_back({Vec2(edge.x, edge.y), Vec2(edge.z, edge.w), tile, nullptr});
        }
    }

    // Merge identical edges in worst case O(N sqrt(N)) time.
    //
    // Let an edge E consist of two points (p,q), and let x~y be a relation defined as |x-y| < e (e = epsilon value):
    //   Two edges A and B are identical, when A.p ~ B.p and A.q ~ B.q, or A.p ~ B.q and A.q ~ B.p.
    //
    // This algorithm first sorts the array of edges (p, q) by p.x. Then, for each edge, look for the other edge which
    // can be merged into this one. This is guaranteed to deduplicate the edges and reduce the size of the array by half.
    //
    // If the array is sorted, checking for the edge which duplicates an edge E will involve a search along the x axis
    // from x = E.p.x. The first vertex of duplicated edge D is guaranteed to lie on the x axis in the range
    // 'E.p.x <= D.p.x < e'. As we have to deal with potentially flipped edges, we may have to search until the
    // x value of the 2nd vertex of D, which makes our range 'E.p.x <= D.p.x < L+2e' (where L is the maximum edge length).
    //
    // Let N be the number of tiles (which have an average number of edges independent of N). Sorting the array costs
    // O(NlogN). Iterating through the array costs O(N) for the outer loop. The inner loop will traverse a distance of
    // L+2e (L being the length of each axis divided by the number of tiles per axis - A/sqrt(N) - and e being constant).
    // Traversing a distance of L is going to iterate through A/L edges, which is O(sqrt(N)). This gives a worst case
    // complexity of O(N sqrt(N)).
    std::cout << "Voronoi: Merging identical edges." << std::endl;
    std::sort(initial_edges.begin(), initial_edges.end(), [](const Edge& a, const Edge& b) -> bool {
        return a.v0.x < b.v0.x;
    });
    Vector<SharedPtr<Edge>> edges;
    auto ShouldMergeEdges = [](const Edge& a, const Edge& b) -> bool {
        return (vecEqual(a.v0, b.v0, VORONOI_EPSILON) && vecEqual(a.v1, b.v1, VORONOI_EPSILON)) ||
               (vecEqual(a.v0, b.v1, VORONOI_EPSILON) && vecEqual(a.v1, b.v0, VORONOI_EPSILON));
    };
    for (int i = 0; i < initial_edges.size(); ++i) {
        for (int j = i + 1; j < initial_edges.size(); ++j) {
            if (ShouldMergeEdges(initial_edges[i], initial_edges[j])) {
                //std::cout << "Merging " << i << " and " << j << " - " << initial_edges[i].d0 << " " << initial_edges[j].d0 << " - " << initial_edges[i].d1 << " " << initial_edges[j].d1 << std::endl;
                SharedPtr<Edge> new_edge = make_shared<Edge>();
                new_edge->v0 = initial_edges[i].v0;
                new_edge->v1 = initial_edges[i].v1;
                new_edge->d0 = initial_edges[i].d0;
                new_edge->d1 = initial_edges[j].d0;
                edges.emplace_back(new_edge);
                break;
            }
        }
    }

    // Populate edge lists inside each tile.
    std::cout << "Voronoi: Populating edge lists." << std::endl;
    for (SharedPtr<Edge>& edge : edges) {
        edge->d0->edges.push_back({edge, false});
        edge->d1->edges.push_back({edge, false});
    }

    // Reorder edges in each tile by their angle relative to the centre of the tile.
    // atan2(y,x) would return -pi from the left and move clockwise, so edges will be ordered west, north, east, south.
    std::cout << "Voronoi: Sorting edges by angle." << std::endl;
    for (auto it = tiles_.begin(); it != tiles_.end(); ++it) {
        auto& tile = *it;

        // Sort edges.
        std::sort(tile.edges.begin(), tile.edges.end(), [&tile](const OrderedEdge& a, const OrderedEdge& b) {
            return tile.edgeAngle(*a.edge) < tile.edgeAngle(*b.edge);
        });
        for (auto& edge : tile.edges) {
            // Set flipped flag.
            double angle0 = tile.vertexAngle(edge.edge->v0);
            double angle1 = tile.vertexAngle(edge.edge->v1);
            double difference = angle1 - angle0;
            // If the difference is negative, or the difference is greater than a half circle (wraparound case), then
            // the vertices need to be flipped.
            edge.flipped = (difference < 0 || difference > 3.14159f);
        }

        // Remove incomplete tiles.
        if (false) {//if (abs(tile.vertexAngle(tile.edges.back().v1()) - tile.vertexAngle(tile.edges.front().v0())) > VORONOI_EPSILON) {
            for (auto& edge : tile.edges) {
                if (edge.edge->d0 == &tile) {
                    edge.edge->d0 = nullptr;
                } else {
                    edge.edge->d1 = nullptr;
                }
            }
            it = tiles_.erase(it);
        } else {
            // Populate corresponding neighbours. It will be the delunay point which is not this tile.
            tile.neighbours.clear();
            tile.neighbours.reserve(tile.edges.size());
            for (auto &edge : tile.edges) {
                tile.neighbours.push_back(edge.edge->d0 == &tile ? edge.edge->d1 : edge.edge->d0);
            }
        }
    }
}

Vector<VoronoiGraph::Tile> &VoronoiGraph::tiles() {
    return tiles_;
}

const Vector<VoronoiGraph::Tile> &VoronoiGraph::tiles() const {
    return tiles_;
}

World::World(int num_points, float size_x, float size_y) : selected_tile_(nullptr) {
    const int relax_count = 20;

    // Seed RNG.
    std::uniform_real_distribution<> dist(0, 1);
    RngType::result_type const seedval = 0xB0BDAB0B; // TODO: get this from somewhere
    rng_.seed(seedval);

    // Generate points for voronoi diagram.
    Vector<Vec2> points;
    for (int i = 0; i < num_points; ++i) {
        points.emplace_back(dist(rng_) * size_x, dist(rng_) * size_y);
    }

    // Build voronoi diagram and relax points using Lloyds Algorithm.
    voronoi::Voronoi voronoi(points, 0, size_x, 0, size_y);
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
        voronoi = voronoi::Voronoi(points, 0, size_x, 0, size_y);
    }

    // Create map.
    map_ = make_unique<VoronoiGraph>(voronoi);
    unclaimed_tiles_.reserve(map_->tiles().size());
    for (auto& tile : map_->tiles()) {
        unclaimed_tiles_.insert(&tile);
    }

    //generateStates(6, 40);
    fillStates(8);
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

    // Highlight selected tile.
    if (selected_tile_) {
        sf::Color selected_color(20, 50, 120, 120);
        sf::Color selected_edge_color(20, 50, 120, 200);
        drawTile(window, *selected_tile_, selected_color);
        drawTileEdge(window, *selected_tile_, selected_edge_color);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(700, 250));
        ImGui::Begin("Selected Tile");
        ImGui::Text("Centre: %f %f", selected_tile_->centre.x, selected_tile_->centre.y);
        for (int i = 0; i < selected_tile_->neighbours.size(); ++i) {
            auto& e = selected_tile_->edges[i];
            auto& n = selected_tile_->neighbours[i];
            float angle1 = (float)selected_tile_->vertexAngle(e.v0());
            float angle2 = (float)selected_tile_->vertexAngle(e.v1());
            if (n != nullptr) {
                ImGui::Text("- Neighbour %d (edge %.0f %.0f (%.1f) to %.0f %.0f (%.1f) diff: %.1f) - centre %.0f %.0f",
                            i, e.v0().x, e.v0().y, angle1, e.v1().x, e.v1().y, angle2, angle2 - angle1, n->centre.x, n->centre.y);
            } else {
                ImGui::Text("- Neighbour %d (edge %.0f %.0f (%.1f) to %.0f %.0f (%.1f) diff: %.1f) - none",
                            i, e.v0().x, e.v0().y, angle1, e.v1().x, e.v1().y, angle2, angle2 - angle1);
            }
        }
        ImGui::End();
    }
}

void World::drawTile(sf::RenderWindow* window, const VoronoiGraph::Tile& tile, sf::Color colour) {
    sf::VertexArray voronoi_site(sf::Triangles, tile.edges.size() * 3);
    for (int i = 0; i < tile.edges.size(); ++i) {
        voronoi_site[i * 3 + 0].position = toSFML(tile.centre);
        voronoi_site[i * 3 + 0].color = colour;
        voronoi_site[i * 3 + 1].position = toSFML(tile.edges[i].v0());
        voronoi_site[i * 3 + 1].color = colour;
        voronoi_site[i * 3 + 2].position = toSFML(tile.edges[i].v1());
        voronoi_site[i * 3 + 2].color = colour;
    }
    window->draw(voronoi_site);
}

void World::drawTileEdge(sf::RenderWindow *window, const VoronoiGraph::Tile &tile, sf::Color colour) {
    // Convert into list of points.
    Vector<Vec2> ribbon_points;
    ribbon_points.reserve(tile.edges.size());
    for (auto& edge : tile.edges) {
        ribbon_points.push_back(edge.v0());
    }

    // Draw ribbon.
    drawJoinedRibbon(window, ribbon_points, 0.0f, 2.5f, colour);
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

void World::onMouseMove(const Vec2 &projected_mouse_position) {
    float nearest_distance_sq = std::numeric_limits<float>::infinity();
    for (auto& tile : map_->tiles()) {
        float distance = glm::distance2(projected_mouse_position, tile.centre);
        if (distance < nearest_distance_sq) {
            nearest_distance_sq = distance;
            selected_tile_ = &tile;
        }
    }
}

void World::generateStates(int count, int max_size) {
    for (int i = 0; i < count; ++i) {
        std::uniform_int_distribution<> tile_id_dist(0, (int)map_->tiles().size() - 1);

        // Take a tile as the starting land.
        HashSet<Tile*> starting_land = {&map_->tiles()[tile_id_dist(rng_)]};
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
        std::uniform_int_distribution<> tile_id_dist(0, (int)map_->tiles().size() - 1);

        // Take a tile as the starting land.
        HashSet<Tile*> starting_land = {&map_->tiles()[tile_id_dist(rng_)]};
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

bool World::growState(State *state) {
    auto border = state->unorderedBoundary(); // This will always contain a single exclave, the mainland.

    // Map border points to unclaimed land.
    Vector<Tile*> unclaimed_border_tiles;
    for (auto& edge : border[0]) {
        if (unclaimed_tiles_.count(edge->edge->d0) == 1) {
            unclaimed_border_tiles.push_back(edge->edge->d0);
        }
        if (unclaimed_tiles_.count(edge->edge->d1) == 1) {
            unclaimed_border_tiles.push_back(edge->edge->d1);
        }
    }

    // If none are left, give up.
    if (unclaimed_border_tiles.empty()) {
        return false;
    }

    // Claim a random border tile.
    std::uniform_int_distribution<> random_edge_dist(0, (int)unclaimed_border_tiles.size() - 1);
    Tile* next_tile = unclaimed_border_tiles[random_edge_dist(rng_)];
    state->addLandTile(next_tile);
    unclaimed_tiles_.erase(next_tile);
    return true;
}
