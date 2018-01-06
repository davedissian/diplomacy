#pragma once

#include <random>
#include <voronoi/voronoi.h>

enum Direction {
    D_NORTH = 0, D_EAST, D_SOUTH, D_WEST, D_COUNT
};

inline Vec2i getDirectionOffset(Direction d) {
    switch (d) {
        case D_NORTH: return {0, -1};
        case D_EAST: return {1, 0};
        case D_SOUTH: return {0, 1};
        case D_WEST: return {-1, 0};
        default: return {0, 0};
    }
}

inline Vector<HashSet<Vec2i>> getExclaves(HashSet<Vec2i> points) {
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

class World;

class State {
public:
    State(sf::Color colour, const HashMap<String, HashSet<Vec2i>>& land);

    void draw(sf::RenderWindow* window, World* world);
private:
    struct Province {
        HashSet<Vec2i> tiles;
    };
    sf::Color colour_;
    HashMap<String, Province> land_;
};

class VoronoiGraph {
public:
    struct Tile;
    struct Edge {
        Vec2 v0;
        Vec2 v1;
    };

    struct Tile {
        Vec2 centre;

        // Neighbour i is joined at edge i.
        Vector<Edge> edges;
        Vector<Tile*> neighbours;
    };

    explicit VoronoiGraph(const voronoi::Voronoi& voronoi);

    Vector<Tile>& tiles();
    const Vector<Tile>& tiles() const;

private:
    Vector<Tile> tiles_;
};

class World {
public:
    World(int num_points, float size_x, float size_y);

    void draw(sf::RenderWindow* window);
    void drawTile(sf::RenderWindow* window, const VoronoiGraph::Tile& tile, sf::Color colour);

    void onMouseMove(const Vec2& projected_mouse_position);

private:
    HashMap<int, UniquePtr<State>> states_;

    typedef std::mt19937 RngType;
    RngType rng_;
    UniquePtr<VoronoiGraph> map_;
    VoronoiGraph::Tile* selected_tile_;
};