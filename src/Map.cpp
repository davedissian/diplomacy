#include "Common.h"
#include "Map.h"

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
            world->drawTile(window, tile, colour_);
        }
    }
}

World::World() {
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
}

void World::drawTile(sf::RenderWindow* window, Vec2i position, sf::Color colour) {
    // Coordinate system: https://stackoverflow.com/a/15516953

    // Hexagon details.
    float apothem = sqrtf(0.75) * tile_radius;  // a = r / (2 x tan(30))
    const Vec2 x_axis = {tile_radius * 1.5f, apothem};
    const Vec2 y_axis = {0.0f, -apothem * 2.0f};

    // Calculate centre position of hexagon.
    Vec2 tile_position = x_axis * float(position.x) + y_axis * float(position.y);

    // Draw hexagon.
    sf::VertexArray hexagon(sf::TriangleFan, 8);
    hexagon[0].position = toSFML(tile_position);
    hexagon[1].position = toSFML(tile_position + Vec2{tile_radius * 0.5f, apothem});
    hexagon[2].position = toSFML(tile_position + Vec2{tile_radius, 0.0f});
    hexagon[3].position = toSFML(tile_position + Vec2{tile_radius * 0.5f, -apothem});
    hexagon[4].position = toSFML(tile_position + Vec2{-tile_radius * 0.5f, -apothem});
    hexagon[5].position = toSFML(tile_position + Vec2{-tile_radius, 0.0f});
    hexagon[6].position = toSFML(tile_position + Vec2{-tile_radius * 0.5f, apothem});
    hexagon[7].position = toSFML(tile_position + Vec2{tile_radius * 0.5f, apothem});
    for (int i = 0; i < 8; ++i) {
        hexagon[i].color = colour;
    }
    window->draw(hexagon);
}