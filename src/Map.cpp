#include "Common.h"
#include "Map.h"

#include <glm/gtx/vector_query.hpp>
#include <random>
#include <voronoi/voronoi.h>

const float VORONOI_SNAP_EPSILON = 1e-2f;

const float tile_radius = 40.0f; // in pixels.

State::State(sf::Color colour, const HashMap<String, HashSet<Vec2i>>& land) : colour_(colour) {
    // Set up land.
    for (auto& province : land) {
        Province province_data;
        province_data.tiles = province.second;
        land_[province.first] = province_data;
    }

    /*
    // Dump exclaves.
    int grid[10][10];
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            grid[i][j] = 0;
        }
    }
    int exclave_counter = 1;
    for (auto& exclave_pair : land_) {
        for (auto& point : exclave_pair.second.points) {
            grid[point.y][point.x] = exclave_counter;
        }
        for (auto& border_point : exclave_pair.second.border_points) {
            grid[border_point.y][border_point.x] = 9;
        }
        exclave_counter++;
    }
    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            if (grid[y][x] > 0) {
                std::cout << grid[y][x];
            } else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
     */
}

void State::draw(sf::RenderWindow* window, World* world) {
    for (const auto& province : land_) {
        for (const auto& tile : province.second.tiles) {
            //world->drawTile(window, tile, colour_);
        }
    }
}

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
        return (glm::isNull(a.v0 - b.v0, VORONOI_SNAP_EPSILON) && glm::isNull(a.v1 - b.v1, VORONOI_SNAP_EPSILON)) ||
               (glm::isNull(a.v0 - b.v1, VORONOI_SNAP_EPSILON) && glm::isNull(a.v1 - b.v0, VORONOI_SNAP_EPSILON));
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
    for (auto& tile : tiles_) {
        std::sort(tile.edges.begin(), tile.edges.end(), [&tile](const OrderedEdge& a, const OrderedEdge& b) {
            return tile.edgeAngle(*a.edge) < tile.edgeAngle(*b.edge);
        });
        for (auto& edge : tile.edges) {
            // Set corresponding neighbour pointer for this edge. It will be the delunay point which is not this tile.
            tile.neighbours.push_back(edge.edge->d0 == &tile ? edge.edge->d1 : edge.edge->d0);

            // Set flipped flag.
            double angle0 = tile.vertexAngle(edge.edge->v0);
            double angle1 = tile.vertexAngle(edge.edge->v1);
            double difference = angle1 - angle0;
            // If the difference is negative, or the difference is greater than a half circle (wraparound case), then
            // the vertices need to be flipped.
            edge.flipped = (difference < 0 || difference > 3.14159f);
        }
    }
}

Vector<VoronoiGraph::Tile> &VoronoiGraph::tiles() {
    return tiles_;
}

const Vector<VoronoiGraph::Tile> &VoronoiGraph::tiles() const {
    return tiles_;
}

World::World(int num_points, float size_x, float size_y) {
    const int relax_count = 20;

    // Seed RNG.
    std::uniform_real_distribution<> dist(0, 1);
    RngType::result_type const seedval = 1; // TODO: get this from somewhere
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

    states_[10] = make_unique<State>(
            sf::Color::Red, HashMap<String, HashSet<Vec2i>>{
                    {
                            "Mainland", {
                                                {5, 1},
                                                {6, 1},
                                                {7, 1},
                                                {5, 2},
                                                {6, 2},
                                                {7, 2},
                                                {8, 2},
                                                {5, 3},
                                                {6, 3},
                                                {7, 3},
                                                {8, 3},
                                                {6, 4},
                                                {7, 4},
                                                {8, 4}
                                        }
                    },
                    {
                            "Outer Land", {
                                                {1, 1},
                                                {1, 2},
                                                {2, 1}
                                        }
                    }
            }
    );
    states_[20] = make_unique<State>(
            sf::Color::Blue, HashMap<String, HashSet<Vec2i>>{
                    {
                            "Main", {
                                                {2, 2},
                                                {2, 3},
                                                {3, 2}
                                        }
                    }
            }
    );
}

void World::draw(sf::RenderWindow* window) {
    /*
    int world_size = 16;
    for (int x = 0; x < world_size; ++x) {
        for (int y = x / 2; y < world_size + x / 2; ++y) {
            drawTile(window, {x, y}, sf::Color(60, 60, 60));
        }
    }

    // Draw state boundaries.
    for (auto& state : states_) {
        state.second->draw(window, this);
    }
     */

    // Draw map.
    for (auto& tile : map_->tiles()) {
        std::uniform_int_distribution<> colour_dist(40, 70);
        sf::Uint8 colour_value = sf::Uint8(40 + std::hash<float>()(tile.centre.x + tile.centre.y) % 30);
        drawTile(window, tile, sf::Color(colour_value, colour_value, colour_value));
    }

    // Draw selected tile (and neighbours)
    if (selected_tile_) {
        /*
        sf::Color selected_color(20, 50, 120, 120);
        drawTile(window, *selected_tile_, selected_color);
         */

        // Create state.
        HashSet<VoronoiGraph::Tile*> state;
        state.insert(selected_tile_);
        state.insert(selected_tile_->neighbours.begin(), selected_tile_->neighbours.end());

        // Draw state.
        sf::Color state_colour(20, 50, 120, 120);
        for (auto& state_tile : state) {
            drawTile(window, *state_tile, state_colour);
        }

        // Build edge list containing state boundaries.
        Vector<VoronoiGraph::OrderedEdge*> boundaries;
        for (auto& state_tile : state) {
            for (int i = 0; i < state_tile->neighbours.size(); ++i) {
                if (state.count(state_tile->neighbours[i]) == 0) {
                    boundaries.push_back(&state_tile->edges[i]);
                }
            }
        }

        // Sort boundaries by joining vertices together.
        for (int i = 0; i < boundaries.size(); ++i) {
            // Consider all edges after this one.
            for (int j = i + 1; j < boundaries.size(); ++j) {
                // If edge[j] joins the end of edge[i], then move edge[j] to i + 1.
                if (glm::isNull(boundaries[j]->v0() - boundaries[i]->v1(), VORONOI_SNAP_EPSILON)) {
                    auto* temp = boundaries[i + 1];
                    boundaries[i + 1] = boundaries[j];
                    boundaries[j] = temp;
                    break;
                }
            }
        }

        // Convert into list of points.
        Vector<Vec2> ribbon_points;
        ribbon_points.reserve(boundaries.size());
        for (auto& edge : boundaries) {
            ribbon_points.push_back(edge->v0());
        }

        // Draw border using a ribbon.
        if (selected_tile_->edges.size() > 2) {
            sf::VertexArray border(sf::Quads, ribbon_points.size() * 4);
            float inner_thickness = 0.0f;
            float outer_thickness = 5.0f;

            // Generate ribbon edges from the cycle of points.
            Vector<Pair<Vec2, Vec2>> ribbon_edges;
            int num_points = (int)ribbon_points.size();
            for (int i = 0; i < num_points; ++i) {
                // To generate pair of ribbon points about point i, we need to consider points: i-1 -> i -> i+1.
                const Vec2& a = ribbon_points[(i - 1 + num_points) % num_points];
                const Vec2& b = ribbon_points[i];
                const Vec2& c = ribbon_points[(i + 1) % num_points];
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
            sf::Color border_colour(200, 200, 200, 100);
            for (int i = 0; i < ribbon_edges.size(); ++i) {
                border[i * 4].position = toSFML(ribbon_edges[i].first);
                border[i * 4].color = border_colour;
                border[i * 4 + 1].position = toSFML(ribbon_edges[(i + 1) % ribbon_edges.size()].first);
                border[i * 4 + 1].color = border_colour;
                border[i * 4 + 2].position = toSFML(ribbon_edges[(i + 1) % ribbon_edges.size()].second);
                border[i * 4 + 2].color = border_colour;
                border[i * 4 + 3].position = toSFML(ribbon_edges[i].second);
                border[i * 4 + 3].color = border_colour;
            }
            window->draw(border);
        }

        ImGui::Begin("Selected Tile");
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2(500, 500));
        ImGui::Text("Centre: %f %f", selected_tile_->centre.x, selected_tile_->centre.y);
        for (int i = 0; i < selected_tile_->neighbours.size(); ++i) {
            auto& e = selected_tile_->edges[i];
            auto& n = *selected_tile_->neighbours[i];
            ImGui::Text("- Neighbour %d (edge %.0f %.0f to %.0f %.0f) - centre %.0f %.0f",
                        i, e.v0().x, e.v0().y, e.v1().x, e.v1().y, n.centre.x, n.centre.y);
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
