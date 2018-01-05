#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "glm/vec2.hpp"

#include <iostream>

#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>

using String = std::string;

template <typename T>
using Vector = std::vector<T>;

template <typename T>
using Queue = std::queue<T>;

template <typename T>
using HashSet = std::unordered_set<T>;

template <typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template <typename T>
using UniquePtr = std::unique_ptr<T>;

using std::make_unique;

using Vec2i = glm::ivec2;
using Vec2 = glm::vec2;
using Vec3i = glm::ivec3;
using Vec3 = glm::vec3;

using i16 = std::int16_t;
using i32 = std::int32_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using f32 = float;
using f64 = double;

namespace std {
template <> struct hash<Vec2i>
{
    size_t operator()(const Vec2i& x) const
    {
        return hash<i32>()((x.x & 0xFFFF) | ((x.y & 0xFFFF) << 16));
    }
};
}

sf::Vector2f toSFML(const Vec2& v) {
    return sf::Vector2f(v.x, v.y);
}

enum Direction {
    D_NORTH = 0, D_EAST, D_SOUTH, D_WEST, D_COUNT
};

Vec2i getDirectionOffset(Direction d) {
    switch (d) {
    case D_NORTH: return {0, -1};
    case D_EAST: return {1, 0};
    case D_SOUTH: return {0, 1};
    case D_WEST: return {-1, 0};
    default: return {0, 0};
    }
}

Vector<HashSet<Vec2i>> getExclaves(HashSet<Vec2i> points) {
    /*
        Example point set:
        ### ##
        ### ###
        ##   #

        Exclaves are found with the following algorithm:

        1) Remove point A from the input.
        2) Create a new exclave E.
        3) Add A to exclave E.
        4) Do a breadth first search from A, treating immediate neighbours as edges, removing
           from input set when visited.
        5) If more points exist, goto 1.
    */

    // Find exclaves.
    Vector<HashSet<Vec2i>> exclaves;
    while (!points.empty()) {
        // Create exclave.
        exclaves.emplace_back(HashSet<Vec2i>{});
        auto& exclave = exclaves.back();

        // Search for neighbours.
        Queue<Vec2i> bfs_queue;
        bfs_queue.push(*points.begin());
        points.erase(points.begin());
        while (!bfs_queue.empty()) {
            // Dequeue and add to exclave.
            Vec2i p = bfs_queue.front();
            exclave.insert(p);
            bfs_queue.pop();

            // Traverse neighbours.
            for (int d = 0; d < D_COUNT; ++d) {
                Vec2i neighbour = p + getDirectionOffset((Direction)d);
                auto neighbour_it = points.find(neighbour);
                if (neighbour_it != points.end()) {
                    bfs_queue.push(neighbour);
                    points.erase(neighbour_it);
                }
            }
        }
    }
    return exclaves;
}

class Country {
public:
    Country(const HashMap<String, HashSet<Vec2i>>& land);

    void draw(sf::RenderWindow* window);

private:
    struct Exclave {
        HashSet<Vec2i> points;
        Vector<Vec2i> border_points;
    };
    HashMap<String, Exclave> land_;
};

Country::Country(const HashMap<String, HashSet<Vec2i>>& land) {
    // Build exclaves.
    for (auto& exclave_pair : land) {
        Exclave exclave_data;
        exclave_data.points = exclave_pair.second;
        land_[exclave_pair.first] = exclave_data;
    }

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
}

void Country::draw(sf::RenderWindow* window) {

}

class World {
public:
    World();
    void draw(sf::RenderWindow* window);
private:
    std::unordered_map<int, UniquePtr<Country>> countries_;
};

World::World() {
    countries_[10] = make_unique<Country>(
        HashMap<String, HashSet<Vec2i>>{
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
}

void World::draw(sf::RenderWindow* window) {
    // Coordinate system: https://stackoverflow.com/a/15516953
    int world_size = 50;
    float tile_radius = 20.0f; // in pixels.

    Vec2 offset = {tile_radius, 768 - tile_radius};

    // a = r / (2 x tan(30))
    float apothem = sqrt(0.75) * tile_radius;

    // Draw hexagrid
    const Vec2 x_axis = {tile_radius * 1.5f, apothem};
    const Vec2 y_axis = {0.0f, -apothem * 2.0f};
    for (int x = 0; x < world_size; ++x) {
        for (int y = 0; y < world_size; ++y) {
            sf::VertexArray hexagon(sf::TriangleFan, 8);
            Vec2 tile_centre = offset + x_axis * float(x) + y_axis * float(y);
            hexagon[0].color = sf::Color(255, 0, 0);
            hexagon[0].position = toSFML(tile_centre);
            hexagon[1].position = toSFML(tile_centre + Vec2{tile_radius * 0.5f, apothem});
            hexagon[2].position = toSFML(tile_centre + Vec2{tile_radius, 0.0f});
            hexagon[3].position = toSFML(tile_centre + Vec2{tile_radius * 0.5f, -apothem});
            hexagon[4].position = toSFML(tile_centre + Vec2{-tile_radius * 0.5f, -apothem});
            hexagon[5].position = toSFML(tile_centre + Vec2{-tile_radius, 0.0f});
            hexagon[6].position = toSFML(tile_centre + Vec2{-tile_radius * 0.5f, apothem});
            hexagon[7].position = toSFML(tile_centre + Vec2{tile_radius * 0.5f, apothem});
            window->draw(hexagon);
        }
    }
}

void runGame(Vec2i window_size) {
    sf::RenderWindow window(sf::VideoMode(window_size.x, window_size.y), "RTS Game");
    World world;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            world.draw(&window);
            window.display();
        }
    }
};

int main() {
    runGame({1024, 768});

    return 0;
};
