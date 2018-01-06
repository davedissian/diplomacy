#include "Common.h"
#include "Map.h"

#include <glm/gtx/vector_query.hpp>
#include <random>
#include <voronoi/voronoi.h>

const float VORONOI_SNAP_EPSILON = 1e2f * std::numeric_limits<float>::epsilon();

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

VoronoiGraph::VoronoiGraph(const voronoi::Voronoi& voronoi) {
    /*
    for (auto& edge : vsite.edges) {
        edges_.push_back({Vec2(edge.x, edge.y), Vec2(edge.z, edge.w)});
    }
     */

    // Set up tiles.
    //tiles_.reserve(voronoi.sites.size());
    for (auto& site : voronoi.sites) {
        Tile tile;
        tile.centre = site.coord;
        for (auto& edge : site.edges) {
            tile.edges.push_back({Vec2(edge.x, edge.y), Vec2(edge.z, edge.w)});
        }
        tile.neighbours.resize(tile.edges.size());
        tiles_.emplace_back(tile);
    }

    // Set up neighbour pointers via expensive O(n^2) search.
    for (int i = 0; i < tiles_.size(); ++i) {
        for (int j = 0; j < tiles_.size(); ++j) {
            if (i == j) {
                continue;
            }

            // Determine if a pair of edges joins tiles i and j.
            for (int e_i = 0; e_i < tiles_[i].edges.size(); ++e_i) {
                for (int e_j = 0; e_j < tiles_[j].edges.size(); ++e_j) {
                    Edge& edge_i = tiles_[i].edges[e_i];
                    Edge& edge_j = tiles_[j].edges[e_j];
                    if (glm::isNull(edge_i.v0 - edge_j.v0, VORONOI_SNAP_EPSILON) &&
                        glm::isNull(edge_i.v1 - edge_j.v1, VORONOI_SNAP_EPSILON)) {
                        edge_i.v0 = edge_j.v0;
                        edge_i.v1 = edge_j.v1;
                        tiles_[i].neighbours[e_i] = &tiles_[j];
                        tiles_[j].neighbours[e_j] = &tiles_[i];
                    }
                    if (glm::isNull(edge_i.v0 - edge_j.v1, VORONOI_SNAP_EPSILON) &&
                        glm::isNull(edge_i.v1 - edge_j.v0, VORONOI_SNAP_EPSILON)) {
                        edge_i.v0 = edge_j.v1;
                        edge_i.v1 = edge_j.v0;
                        tiles_[i].neighbours[e_i] = &tiles_[j];
                        tiles_[j].neighbours[e_j] = &tiles_[i];
                    }
                }
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
        sf::Color selected_color(20, 50, 120, 120);
        sf::Color selected_neighbour_colour = selected_color;
        selected_neighbour_colour.a /= 3;
        drawTile(window, *selected_tile_, selected_color);
        for (auto& neighbour : selected_tile_->neighbours) {
            if (neighbour) {
                drawTile(window, *neighbour, selected_neighbour_colour);
            }
        }

        ImGui::Begin("Selected Tile");
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2(500, 500));
        ImGui::Text("Centre: %f %f", selected_tile_->centre.x, selected_tile_->centre.y);
        for (int i = 0; i < selected_tile_->neighbours.size(); ++i) {
            auto& e = selected_tile_->edges[i];
            if (selected_tile_->neighbours[i]) {
                auto& n = *selected_tile_->neighbours[i];
                ImGui::Text("- Neighbour %d (edge %.1f %.1f to %.1f %.1f) - centre %.1f %.1f", i, e.v0.x, e.v0.y, e.v1.x, e.v1.y, n.centre.x, n.centre.y);
            } else {
                ImGui::Text("- Neighbour %d (edge %.1f %.1f to %.1f %.1f) - null", i, e.v0.x, e.v0.y, e.v1.x, e.v1.y);
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
        voronoi_site[i * 3 + 1].position = toSFML(tile.edges[i].v0);
        voronoi_site[i * 3 + 1].color = colour;
        voronoi_site[i * 3 + 2].position = toSFML(tile.edges[i].v1);
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
